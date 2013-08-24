package d3scomp.beeclickarmj;

import java.io.IOException;

public class Test {

	public static void main(String[] args) throws CommException, IOException {
		Comm comm = new JSSCComm("COM11");
		
		comm.setAddr(0xBABA, 0x0102);
		comm.setChannel(2);
		
		System.out.println("Started. Press ENTER to exit.");
		System.in.read();
		
		comm.shutdown();
	}

}
