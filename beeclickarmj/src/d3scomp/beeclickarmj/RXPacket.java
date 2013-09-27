package d3scomp.beeclickarmj;

public class RXPacket {
	private byte[] data;
	private int rssi;
	private int lqi;
	private int fcs;
	
	public RXPacket(byte[] data, int rssi, int lqi, int fcs) {
		this.data = data;
		this.fcs = fcs;
		this.lqi = lqi;
		this.rssi = rssi;
	}
	
	public byte[] getData() {
		return data;
	}
	
	public int getFCS() {
		return fcs;
	}
	
	public int getLQI() {
		return lqi;
	}
	
	public int getRSSI() {
		return rssi;
	}
}
