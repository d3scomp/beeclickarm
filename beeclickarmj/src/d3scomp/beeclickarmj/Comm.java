package d3scomp.beeclickarmj;

public interface Comm {

	public void shutdown();

	public void broadcastPacket(TXPacket pkt) throws CommException;

	public void setChannel(int channelNo) throws CommException;
	
	public void setAddr(int panId, int sAddr) throws CommException;

	public RXPacket receivePacket() throws CommException;

}