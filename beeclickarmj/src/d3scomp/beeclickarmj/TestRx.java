package d3scomp.beeclickarmj;

import java.io.IOException;
import java.util.Date;

public class TestRx {
	public static void main(String[] args) throws CommException, IOException, InterruptedException {
		System.out.println("TestRX");
		System.out.println("======");
		
		Comm comm = new JSSCComm("COM7");

		comm.start();

		comm.setAddr(0xBABA, 0x0103);
		comm.setChannel(0);
		
		comm.setTemperatureReadingListener(new TemperatureReadingListener() {
			@Override
			public void readTemperature(float temperature) {
				System.out.format("Temperature reading: %.1f°C%n", temperature);
			}
		});
		
		comm.setHumidityReadingListener(new HumidityReadingListener() {
			@Override
			public void readHumidity(float humidity) {
				System.out.format("Humidity reading listener %.1f%%%n", humidity);
			}
		});
		
		comm.setGPSReadingListener(new GPSReadingListener() {
			@Override
			public void readGPS(double longtitude, double lattitude, Date time) {
				System.out.format("GPS reading listener lat: %f, lon: %f, time: %s%n", lattitude, longtitude, time.toString());
			}
		});
		
		comm.setReceivePacketListener(new ReceivePacketListener() {
			@Override
			public void receivePacket(RXPacket packet) {
				int len = packet.getData().length;
				byte[] data = packet.getData();
				System.out.format("Packet received: len=%d srcPanId=%04x srcSAddr=%04x fcs=%04x lqi=%d rssi=%d ... \"%s\"\n", len, packet.getSrcPanId(), packet.getSrcSAddr(), packet.getFCS(), packet.getLQI(), packet.getRSSI(), new String(data));				
			}
		});
		
		
		
		System.out.println("Running, Press ENTER to exit.");
		System.in.read();

		comm.shutdown();
	}
}
