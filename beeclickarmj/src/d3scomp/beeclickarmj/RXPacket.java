package d3scomp.beeclickarmj;

public class RXPacket {
	private byte[] data;
	private int srcPanId;
	private int srcSAddr;
	private int rssi;
	private int lqi;
	private int fcs;
	
	public RXPacket(byte[] data, int srcPanId, int srcSAddr, int rssi, int lqi, int fcs) {
		this.data = data;
		this.srcPanId = srcPanId;
		this.srcSAddr = srcSAddr;
		this.fcs = fcs;
		this.lqi = lqi;
		this.rssi = rssi;
	}
	
	public byte[] getData() {
		return data;
	}
	
	public int getSrcPanId() {
		return srcPanId;
	}

	public int getSrcSAddr() {
		return srcSAddr;
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
