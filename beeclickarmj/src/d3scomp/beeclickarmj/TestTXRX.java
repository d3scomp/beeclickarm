package d3scomp.beeclickarmj;

import java.io.IOException;

public class TestTXRX {

	public static void main(String[] args) throws CommException, IOException, InterruptedException {
		Comm comm = new JSSCComm(args[0]); // e.g. COM5

		comm.setReceivePacketListener(new ReceivePacketListener() {
			
			@Override
			public void receivePacket(RXPacket rxPacket) {
				int len = rxPacket.getData().length;
				byte[] data = rxPacket.getData();
				
				System.out.format("Packet received: len=%d srcPanId=%04x srcSAddr=%04x fcs=%04x lqi=%d rssi=%d ... \"%s\"\n", len, rxPacket.getSrcPanId(), rxPacket.getSrcSAddr(), rxPacket.getFCS(), rxPacket.getLQI(), rxPacket.getRSSI(), new String(data));
			}
		});

		comm.start();
		
		comm.setAddr(0xBABA, 0x0102);
		comm.setChannel(1);
		
		System.out.println("Initialized. Press ENTER to start sending.");
		do {
			System.in.read();
		} while (System.in.available() > 0);
		System.out.format("Sending...\n");

		long nanoTimeStart = System.nanoTime();
		
		TXPacket txPacket = null;
		int i;
		for (i=0; i<1000; i++) {
			txPacket = new TXPacket(String.format("SR (A)! SR (B)! SR (C)! SR (D)! SR (E)! SR (F)! SR (G)! SR (H)! SR (I)! SR (J)! SR (K)! SR (L)! SR (M)! SR (N)! [%04d]", i).getBytes());
//			txPacket = new TXPacket(String.format("[%04d]", i).getBytes());
			comm.broadcastPacket(txPacket);
		}
		
		txPacket.waitForNotPendingState();

		long nanoTimeEnd = System.nanoTime();
		double duration = (nanoTimeEnd - nanoTimeStart) / 1000000000.0;

		System.out.format("Done sending.\n");
		System.out.format("Took %f s (%f ms/msg, %f msg/s). Avg. throughput %f kbps.\n", duration, duration / i * 1000, i / duration, i * 118 * 8 / duration / 1000);
//		System.out.format("Took %f s (%f ms/msg, %f msg/s). Avg. throughput %f kbps.\n", duration, duration / i * 1000, i / duration, i * 6 * 8 / duration / 1000);
		System.out.format("Press ENTER to exit.\n");
		do {
			System.in.read();
		} while (System.in.available() > 0);

		comm.shutdown();
	}

}
