package d3scomp.beeclickarmj;

import java.nio.ByteBuffer;
import java.util.concurrent.LinkedBlockingQueue;

import jssc.SerialPort;
import jssc.SerialPortException;


public class TXRX {
	private LinkedBlockingQueue<TODMsg> todQueue;
	private SerialPort port;
	
	private boolean stopRequested = false;
	
	private Thread txThread, rxThread;
	
	public TXRX(String portName) throws SerialPortException {
		port = new SerialPort(portName);
		
		port.openPort();
		port.setParams(921600, 8, 1, 0, false, false);
		
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
	}
	
	public void shutdown() {
		stopRequested = true;

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
		while (!stopRequested) {
			try {
				TODMsg msg = todQueue.take();

				ByteBuffer txBuf = ByteBuffer.allocate(msg.getSize());
				
				msg.write(txBuf);
				
				try {
					port.writeBytes(txBuf.array());
				} catch (SerialPortException e) {
					e.printStackTrace();
				}
			} catch (InterruptedException e) {
			}
		}
	}
	
	private void rxRun() {
		
	}
}
