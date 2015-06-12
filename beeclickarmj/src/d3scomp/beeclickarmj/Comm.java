package d3scomp.beeclickarmj;

public interface Comm {

	public void shutdown();

	public void broadcastPacket(TXPacket pkt) throws CommException;

	public void setChannel(int channelNo) throws CommException;

	/**
	 * Sets output Tx power in dB
	 * 
	 * Supported range is from -36.3 dB to 0 dB
	 * 
	 * @param power
	 *            Requested Tx power in dB
	 * @throws CommException
	 */
	public void setTxPower(float power) throws CommException;

	public void setAddr(int panId, int sAddr) throws CommException;
	
	public RXPacket receivePacket() throws CommException;

	public void setReceivePacketListener(ReceivePacketListener listener);

	public abstract void start() throws CommException;

}