package d3scomp.beeclickarmj;

import java.io.IOException;

public class Test {

	public static void main(String[] args) throws CommException, IOException, InterruptedException {
		Comm comm = new JSSCComm("COM5");
		
		comm.start();
		
		comm.setAddr(0xBABA, 0x0102);
		comm.setChannel(2);
		
		for (int i=1; i<10; i++) {
			TXPacket txPacket = new TXPacket(String.format("Sai Ram! [%d]", i).getBytes());
			comm.broadcastPacket(txPacket);

			System.out.print("Packet " + i + ": ");
			txPacket.waitForNotPendingState();
			System.out.println(txPacket.getStatus());
		}
		
		System.out.println("Done but still running. Press ENTER to exit.");
		System.in.read();
		
		comm.shutdown();
	}

}
