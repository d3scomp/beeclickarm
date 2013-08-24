package d3scomp.beeclickarmj;

public class RXPacket {
	byte[] data;
	int rssi;
	int lqi;
	byte fcs;
	
	public RXPacket(byte[] data, int rssi, int lqi, byte fcs) {
		this.data = data;
		this.fcs = fcs;
		this.lqi = lqi;
		this.rssi = rssi;
	}
	
	public byte[] getData() {
		return data;
	}
	
	public byte getFCS() {
		return fcs;
	}
	
	public int getLQI() {
		return lqi;
	}
	
	public int getRSSI() {
		return rssi;
	}
}
