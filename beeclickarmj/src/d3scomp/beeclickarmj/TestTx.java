package d3scomp.beeclickarmj;

import java.io.IOException;

public class TestTx {
	public static void main(String[] args) throws CommException, IOException, InterruptedException {
		System.out.println("TestTX");
		System.out.println("======");
		
		Comm comm = new JSSCComm("COM7");

		comm.start();

		comm.setAddr(0xBABA, 0x0102);
		comm.setChannel(0);
		
		TXPacket txPacket = null;
		for (int i=0; i<10000000; i++) {
			float power = 0;
			if(i % 2 == 0)
				power = -36.3f;
			
			comm.setTxPower(power);
			
			txPacket = new TXPacket(String.format("SR! [%d] TxPower:%f", i, power).getBytes());
			comm.broadcastPacket(txPacket);
			System.out.print("Packet " + i + " sent.\n");
//			txPacket.waitForNotPendingState();
//			System.out.println(txPacket.getStatus());
		}
		
		txPacket.waitForNotPendingState();

		System.out.println("Done but still running. Press ENTER to exit.");
		System.in.read();

		comm.shutdown();
	}

}
