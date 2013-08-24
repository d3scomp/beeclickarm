package d3scomp.beeclickarmj;

import jssc.SerialPort;
import jssc.SerialPortException;


public class JSSCComm extends AbstractComm {
	protected String portName;
	protected SerialPort port;
	
	public JSSCComm(String portName) {
		this.portName = portName;
	}
	
	@Override
	protected void openPort() throws CommException {
		try {
			port = new SerialPort(portName);

			port.openPort();
			port.setParams(921600, 8, 1, 0, false, false);
//			port.setParams(2400, 8, 1, 0, false, false);		
		} catch (SerialPortException e) {
			throw new CommException("Error establishing the serial communication.", e);
		}
	}
	
	@Override
	protected void writePort(byte[] buffer) throws CommException {
		try {
			port.writeBytes(buffer);
		} catch (SerialPortException e) {
			throw new CommException("Error writing serial port.", e);
		}
	}
	
	@Override
	protected byte[] readPort(int size) throws InterruptedException, CommException {
		try {
			Thread currentThread = Thread.currentThread();

			while (port.getInputBufferBytesCount() < size && !currentThread.isInterrupted()) {
				Thread.sleep(1);
			}

			if (currentThread.isInterrupted()) {
				throw new InterruptedException();
			} else {
				return port.readBytes(size);
			}
		} catch (SerialPortException e) {
			throw new CommException("Error reading serial port.", e);
		}
	}
	
	@Override
	protected void closePort() throws CommException {
		try {
			port.closePort();
		} catch (SerialPortException e) {
			throw new CommException("Error when closing the port.", e);
		}
	}
}
