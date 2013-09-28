package d3scomp.beeclickarmj;

import java.io.IOException;

public class TestRx {
	public static void main(String[] args) throws CommException, IOException, InterruptedException {
		System.out.println("TestRX");
		System.out.println("======");
		
		Comm comm = new JSSCComm("COM8");

		comm.start();

		comm.setAddr(0xBABA, 0x0103);
		comm.setChannel(1);

		for (int i=0; i<1000; i++) {
			RXPacket rxPacket = comm.receivePacket();
			int len = rxPacket.getData().length;
			byte[] data = rxPacket.getData();
			
			System.out.format("Packet received: len=%d srcPanId=%04x srcSAddr=%04x fcs=%04x lqi=%d rssi=%d ... \"%s\"\n", len, rxPacket.getSrcPanId(), rxPacket.getSrcSAddr(), rxPacket.getFCS(), rxPacket.getLQI(), rxPacket.getRSSI(), new String(data));
		}

		System.out.println("Done but still running. Press ENTER to exit.");
		System.in.read();

		comm.shutdown();
	}

}
