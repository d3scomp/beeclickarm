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
	
	/**
	 * Sets listener which is used when new temperature reading is acquired
	 */
	public void setTemperatureReadingListener(TemperatureReadingListener listener);
	
	/**
	 * Sets listener which is used when new humidity reading is acquired
	 */
	public void setHumidityReadingListener(HumidityReadingListener listener);
	
	/**
	 * Sets GPS reading listener which is used when new GPS reading is acquired from device
	 */
	public void setGPSReadingListener(GPSReadingListener listener);

	public abstract void start() throws CommException;

}