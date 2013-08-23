package d3scomp.beeclickarmj;

import jssc.SerialPort;
import jssc.SerialPortException;
import jssc.SerialPortList;

public class Test {

	public static void main(String[] args) throws SerialPortException {
		String[] portNames = SerialPortList.getPortNames();
		
		for (String portName : portNames) {
			System.out.format("%s\n", portName);
		}

		TXRX txrx = new TXRX("COM11");
		
		
		
		txrx.shutdown();
	}

}
