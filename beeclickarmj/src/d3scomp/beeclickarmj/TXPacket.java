package d3scomp.beeclickarmj;

public class TXPacket {
	public enum Status {
		NONE, PENDING, SENT, ERROR
	}
	
	private byte[] data;
	private Status status;

	public TXPacket(byte[] data) {
		this.data = data;
		this.status = Status.NONE;
	}
	
	public Status getStatus() {
		return status;
	}
	
	public void setStatus(Status status) {
		this.status = status;
	}
	
	public byte[] getData() {
		return data;
	}
	
	public synchronized void waitForNotPendingState() throws InterruptedException {
		while (status == Status.PENDING) {
			wait();
		}
	}
}
