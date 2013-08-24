package d3scomp.beeclickarmj;

import java.nio.ByteBuffer;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.BrokenBarrierException;
import java.util.concurrent.CyclicBarrier;
import java.util.concurrent.LinkedBlockingQueue;

import jssc.SerialPort;
import jssc.SerialPortException;


public class JSSCComm implements Comm {
	private LinkedBlockingQueue<TODMsg> todQueue = new LinkedBlockingQueue<TODMsg>();

	private SerialPort port;
	
	private Thread txThread, rxThread;
	
	private boolean isOperational;
	
	public JSSCComm(String portName) throws CommException {
		try {
			port = new SerialPort(portName);
			
			port.openPort();
			port.setParams(921600, 8, 1, 0, false, false);
			
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
			
		} catch (SerialPortException e) {
			throw new CommException("Error establishing the serial communication.", e);
		}
		
		isOperational = true;
	}
	
	/* (non-Javadoc)
	 * @see d3scomp.beeclickarmj.Comm#shutdown()
	 */
	@Override
	public void shutdown() {
		isOperational = false;
		
		txThread.interrupt();
		rxThread.interrupt();

		try {
			txThread.join();
			rxThread.join();
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
		
		try {
			port.closePort();
		} catch (SerialPortException e) {
			e.printStackTrace();
		}
	}
	
	private void txRun() {	
		try {
			while (!txThread.isInterrupted()) {
				TODMsg msg;
				msg = todQueue.take();
				ByteBuffer txBuf = ByteBuffer.allocate(msg.getSize());

				msg.write(txBuf);

				port.writeBytes(txBuf.array());
			}
		} catch (InterruptedException e) {
		} catch (SerialPortException e) {
			e.printStackTrace();
			System.out.println("Serial communication exception occured. Shutting down the communication stack.");
			shutdown();
		}

	}

	
	private ByteBuffer rxBuf = ByteBuffer.allocate(TOHMsg.MAX_MSG_SIZE);
	
	private void rxRun() {
		try {
			System.out.println("Waiting for SYNC.");

			rxBuf.put(port.readBytes(1)[0]);

			int pos;
			while ((pos = rxBuf.position()) != TOHMsg.Sync.correctSyncBytes.length) {
				if (rxBuf.get(pos - 1) != TOHMsg.Sync.correctSyncBytes[pos - 1]) {
					rxBuf.clear();
				}
				
				rxBuf.put(port.readBytes(1)[0]);
			}
			
			System.out.println("SYNC succesful.");
			
			while (!rxThread.isInterrupted()) {
				rxBuf.clear();

				byte msgType = port.readBytes(1)[0];
				rxBuf.put(msgType);
				
				int bytesStilExpected;					
				while ((bytesStilExpected = TOHMsg.getExpectedSizeLowerBound(rxBuf) - rxBuf.position()) > 0) {
					rxBuf.put(port.readBytes(bytesStilExpected));
				}
					
				rxBuf.flip();
				TOHMsg msg = TOHMsg.read(rxBuf);
				rxHandle(msg);
			}
		} catch (SerialPortException e) {
			e.printStackTrace();
			System.out.println("Serial communication exception occured. Shutting down the communication stack.");
			shutdown();
		}
	}
	
	private void ensureOperational() throws CommException {
		if (!isOperational) {
			throw new CommException("Communication stack is not operational."); 
		}
	}
	
	private int txSeq;
	private Map<Integer, TXPacket> txPackets = Collections.synchronizedMap(new HashMap<Integer, TXPacket>());
	
	/* (non-Javadoc)
	 * @see d3scomp.beeclickarmj.Comm#broadcastPacket(d3scomp.beeclickarmj.TXPacket)
	 */
	@Override
	public synchronized void broadcastPacket(TXPacket pkt) throws CommException {
		ensureOperational();
		
		txPackets.put(txSeq, pkt);
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
	
	/* (non-Javadoc)
	 * @see d3scomp.beeclickarmj.Comm#setChannel(int)
	 */
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

	
	private int panIdRequested = -1, sAddrRequested = -1;
	private CyclicBarrier addrSync = new CyclicBarrier(2);

	/* (non-Javadoc)
	 * @see d3scomp.beeclickarmj.Comm#setAddr(int, int)
	 */
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
	
	

	private LinkedBlockingQueue<RXPacket> rxQueue = new LinkedBlockingQueue<RXPacket>();

	/* (non-Javadoc)
	 * @see d3scomp.beeclickarmj.Comm#receivePacket()
	 */
	@Override
	public RXPacket receivePacket() throws CommException {
		ensureOperational();
		
		try {
			return rxQueue.take();
			
		} catch (InterruptedException e) {
			e.printStackTrace();
			return null;
		}
	}

	
	private void rxHandle(TOHMsg msg) {
		if (msg.type == TOHMsg.Type.PACKET_SENT) {
			TOHMsg.PacketSent tmsg = (TOHMsg.PacketSent)msg;
			
			TXPacket pkt = txPackets.remove(tmsg.seq);
			pkt.setStatus(tmsg.status == 0 ? TXPacket.Status.SENT : TXPacket.Status.ERROR);
		
		} if (msg.type == TOHMsg.Type.RECV_PACKET) {
				TOHMsg.RecvPacket tmsg = (TOHMsg.RecvPacket)msg;
				
				RXPacket pkt = new RXPacket(tmsg.data, tmsg.rssi, tmsg.lqi, tmsg.fcs);
				try {
					rxQueue.put(pkt);
				} catch (InterruptedException e) {
					e.printStackTrace();
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

		} else if (msg.type == TOHMsg.Type.INFO) {
			TOHMsg.Info tmsg = (TOHMsg.Info)msg;
			System.out.println("===== INFO =====");
			System.out.println(tmsg.text);
			System.out.println("================");
		}
	}
}
