package d3scomp.beeclickarmj;

import java.nio.ByteBuffer;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.LinkedBlockingQueue;

import d3scomp.beeclickarmj.TODMsg.GetHumidity;
import d3scomp.beeclickarmj.TODMsg.GetTemperature;
import d3scomp.beeclickarmj.TOHMsg.Humidity;
import d3scomp.beeclickarmj.TOHMsg.Temperature;

public abstract class AbstractComm implements Comm {
	final static int MAXIMUM_TX_PACKET_IN_QUEUE = 3;
	final static int MAXIMUM_EMPIRICAL_PACKET_DATA_LENGTH = 118;
	
	protected abstract byte[] readPort(int size) throws InterruptedException, CommException;
	protected abstract void writePort(byte[] buffer) throws InterruptedException, CommException;
	protected abstract void openPort() throws CommException;
	protected abstract void closePort() throws CommException;

	private Thread txThread;
	private Thread rxThread;

	private boolean isOperational;

	private LinkedBlockingQueue<TODMsg> todQueue = new LinkedBlockingQueue<TODMsg>();

	@Override
	public void start() throws CommException {
		openPort();

		try {
			todQueue.put(new TODMsg.Sync());
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		txThread = new Thread() {
			public void run() {
				txRun();
			}
		};
		txThread.start();

		rxThread = new Thread() {
			public void run() {
				rxRun();
			}
		};
		rxThread.start();

		isOperational = true;
	}

	@Override
	public void shutdown() {
		txThread.interrupt();
		rxThread.interrupt();

		isOperational = false;

		try {
			txThread.join();
			rxThread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		try {
			closePort();
		} catch (CommException e) {
			e.printStackTrace();
		}
	}

	private void txRun() {	
		try {
			while (true) {
				TODMsg msg;
				msg = todQueue.take();
				ByteBuffer txBuf = ByteBuffer.allocate(msg.getSize());

				msg.write(txBuf);

				writePort(txBuf.array());
			}
		} catch (InterruptedException e) {
		} catch (CommException e) {
			e.printStackTrace();
			System.out.println("Serial communication exception occured. Shutting down the communication stack.");
			shutdown();
		}

	}

	
	private ByteBuffer rxBuf = ByteBuffer.allocate(TOHMsg.MAX_MSG_SIZE);

	private void rxRun() {
		try {
			System.out.println("Waiting for SYNC.");

			rxBuf.put(readPort(1)[0]);

			int pos;
			while ((pos = rxBuf.position()) != TOHMsg.Sync.correctSyncBytes.length) {
				if (rxBuf.get(pos - 1) != TOHMsg.Sync.correctSyncBytes[pos - 1]) {
					rxBuf.clear();
				}

				rxBuf.put(readPort(1)[0]);
			}

			System.out.println("SYNC succesful.");

			while (true) {
				rxBuf.clear();

				byte msgType = readPort(1)[0];
				rxBuf.put(msgType);

				int bytesStilExpected;					
				while ((bytesStilExpected = TOHMsg.getExpectedSizeLowerBound(rxBuf) - rxBuf.position()) > 0) {
					rxBuf.put(readPort(bytesStilExpected));
				}

				rxBuf.flip();
				TOHMsg msg = TOHMsg.read(rxBuf);
				rxHandle(msg);
			}
		} catch (CommException e) {
			e.printStackTrace();
			System.out.println("Serial communication exception occured. Shutting down the communication stack.");
			shutdown();
		} catch (InterruptedException e) {
		}
	}

	private void ensureOperational() throws CommException {
		if (!isOperational) {
			throw new CommException("Communication stack is not operational."); 
		}
	}

	private Map<Integer, TXPacket> txPackets = new HashMap<Integer, TXPacket>();
	private int txSeq;

	@Override
	public void broadcastPacket(TXPacket pkt) throws CommException {
		ensureOperational();

		if (pkt.getData().length > MAXIMUM_EMPIRICAL_PACKET_DATA_LENGTH) {
			throw new CommException("Packet data length exceeds limit of " + MAXIMUM_EMPIRICAL_PACKET_DATA_LENGTH + " bytes.");
		}
		
		synchronized (txPackets) {
			while (txPackets.size() > MAXIMUM_TX_PACKET_IN_QUEUE) { // MAX_QUEUE_LENGTH on device side is 8, so this should be safe bound
				try {
					txPackets.wait();
				} catch (InterruptedException e) {
					e.printStackTrace();
					return;
				}
			}

			txPackets.put(txSeq, pkt);
		}
		pkt.setStatus(TXPacket.Status.PENDING);

		TODMsg.SendPacket msg = new TODMsg.SendPacket();

		msg.seq = txSeq++;
		msg.data = pkt.getData();

		try {
			todQueue.put(msg);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}


	private int channelNoRequested = -1;
	private CyclicBarrier channelNoSync = new CyclicBarrier(2);

	@Override
	public void setChannel(int channelNo) throws CommException {
		if (channelNo < 0 || channelNo > 15) {
			throw new CommException("Channel must be within 0 .. 15");
		}

		if (channelNoRequested != -1) {
			throw new CommException("Another channel set request already in progress.");
		}

		ensureOperational();

		TODMsg.SetChannel msg = new TODMsg.SetChannel();

		msg.channel = channelNo;
		channelNoRequested = channelNo;

		try {
			todQueue.put(msg);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		try {
			channelNoSync.await();
		} catch (InterruptedException | BrokenBarrierException e) {
			e.printStackTrace();
		}

		channelNoRequested = -1;
	}
	
	@Override
	public void setTxPower(float power) throws CommException {
		if(power > 0 || power < -36.3) {
			throw new CommException("Tx power must be within 0 and -36.3 dB");
		}
		ensureOperational();

		TODMsg.SetPower msg = new TODMsg.SetPower();

		msg.power = (short) (power * 10);

		try {
			todQueue.put(msg);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
	
	@Override
	public void getTemperature() {
		TODMsg.GetTemperature msg = new GetTemperature();
		
		try {
			todQueue.put(msg);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	@Override
	public void getHumidity() {
		TODMsg.GetHumidity msg = new GetHumidity();
		
		try {
			todQueue.put(msg);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}


	private int panIdRequested = -1;
	private int sAddrRequested = -1;
	private CyclicBarrier addrSync = new CyclicBarrier(2);

	@Override
	public void setAddr(int panId, int sAddr) throws CommException {
		if (panId < 0 || panId > 0xFFFF) {
			throw new CommException("panId must be within 0x0000 .. 0xFFFF");
		}

		if (sAddr < 0 || sAddr > 0xFFFF) {
			throw new CommException("sAddr must be within 0x0000 .. 0xFFFF");
		}

		if (panIdRequested != -1 || sAddrRequested != -1) {
			throw new CommException("Another addr set request already in progress.");
		}

		ensureOperational();

		TODMsg.SetAddr msg = new TODMsg.SetAddr();

		msg.panId = panId;
		msg.sAddr = sAddr;
		panIdRequested = panId;
		sAddrRequested = sAddr;

		try {
			todQueue.put(msg);
		} catch (InterruptedException e) {
			e.printStackTrace();
		}

		try {
			addrSync.await();
		} catch (InterruptedException | BrokenBarrierException e) {
			e.printStackTrace();
		}

		panIdRequested = -1;
		sAddrRequested = -1;
	}

	protected AbstractComm() {
		super();
	}


	private LinkedBlockingQueue<RXPacket> rxQueue = new LinkedBlockingQueue<RXPacket>();

	@Override
	public RXPacket receivePacket() throws CommException {
		if (receivePacketListener != null) {
			throw new CommException("Receive Packet Listener has been registered. This method should not be called.");
		}
		
		ensureOperational();

		try {
			return rxQueue.take();

		} catch (InterruptedException e) {
			e.printStackTrace();
			return null;
		}
	}

	private ReceivePacketListener receivePacketListener;

	@Override
	public void setReceivePacketListener(ReceivePacketListener receivePacketListener) {
		this.receivePacketListener = receivePacketListener;
	}

	private void rxHandle(TOHMsg msg) {
		if (msg.type == TOHMsg.Type.PACKET_SENT) {
			TOHMsg.PacketSent tmsg = (TOHMsg.PacketSent)msg;

			TXPacket pkt;
			
			synchronized (txPackets) {
				pkt = txPackets.remove(tmsg.seq);
				txPackets.notifyAll();
			}
			
			synchronized (pkt) {
				pkt.setStatus(tmsg.status == 0 ? TXPacket.Status.SENT : TXPacket.Status.ERROR);
				pkt.notifyAll();
			}

		} if (msg.type == TOHMsg.Type.RECV_PACKET) {
			TOHMsg.RecvPacket tmsg = (TOHMsg.RecvPacket)msg;

			RXPacket pkt = new RXPacket(tmsg.data, tmsg.srcPanId, tmsg.srcSAddr, tmsg.rssi, tmsg.lqi, tmsg.fcs);
			
			if (receivePacketListener != null) {
				receivePacketListener.receivePacket(pkt);
			} else {
				try {
					rxQueue.put(pkt);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
			}

		} else if (msg.type == TOHMsg.Type.CHANNEL_SET) {
			TOHMsg.ChannelSet tmsg = (TOHMsg.ChannelSet)msg;

			assert(channelNoRequested == tmsg.channel);

			try {
				channelNoSync.await();
			} catch (InterruptedException | BrokenBarrierException e) {
				e.printStackTrace();
			}

		} else if (msg.type == TOHMsg.Type.ADDR_SET) {
			TOHMsg.AddrSet tmsg = (TOHMsg.AddrSet)msg;

			assert(panIdRequested == tmsg.panId);
			assert(sAddrRequested == tmsg.sAddr);

			try {
				addrSync.await();
			} catch (InterruptedException | BrokenBarrierException e) {
				e.printStackTrace();
			}

		} else if (msg.type == TOHMsg.Type.GPS) {
			TOHMsg.GPS tmsg = (TOHMsg.GPS)msg;
			System.out.println("GPS: " + tmsg.text);
			// TODO: API for GPS coordinates 

		} else if (msg.type == TOHMsg.Type.INFO) {
			TOHMsg.Info tmsg = (TOHMsg.Info)msg;
			System.out.println("======= INFO =======");
			System.out.println(tmsg.text);
			System.out.println("====================");
		} else if (msg.type == TOHMsg.Type.TEMPERATURE) {
			TOHMsg.Temperature tmsg = (TOHMsg.Temperature) msg;
			System.out.println("==== TEMPERATURE ===");
			System.out.println(tmsg.temperature);
			System.out.println("====================");
		} else if (msg.type == TOHMsg.Type.HUMIDITY) {
			TOHMsg.Humidity tmsg = (TOHMsg.Humidity) msg;
			System.out.println("===== HUMIDITY =====");
			System.out.println(tmsg.humidity);
			System.out.println("====================");
		}
	}
}