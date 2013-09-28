package d3scomp.beeclickarmj;

import java.io.IOException;

public class TestTx {
	public static void main(String[] args) throws CommException, IOException, InterruptedException {
		System.out.println("TestTX");
		System.out.println("======");
		
		Comm comm = new JSSCComm("COM5");

		comm.start();

		comm.setAddr(0xBABA, 0x0102);
		comm.setChannel(0);

		TXPacket txPacket = null;
		for (int i=0; i<1000; i++) {
			txPacket = new TXPacket(String.format("SR! [%d]", i).getBytes());
			comm.broadcastPacket(txPacket);
			System.out.print("Packet " + i + "sent.\n");
//			txPacket.waitForNotPendingState();
//			System.out.println(txPacket.getStatus());
		}
		
		txPacket.waitForNotPendingState();

		System.out.println("Done but still running. Press ENTER to exit.");
		System.in.read();

		comm.shutdown();
	}

}
