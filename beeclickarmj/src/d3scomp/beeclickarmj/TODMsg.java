package d3scomp.beeclickarmj;

import java.nio.ByteBuffer;

public abstract class TODMsg {
	static final int typeSize = 1;
	
	Type type;
	
	public static enum Type {
		Sync, SendPacket, SetChannel
	}
	
	public static class Sync extends TODMsg {
		static final byte[] SYNC_PATTERN = {'H', 'e', 'l', 'l', 'o', ' ', 't', 'h', 'e', 'r', 'e', '!'};
		byte pattern[] = SYNC_PATTERN;
		
		public Sync() {
			type = Type.Sync;
		}
		
		protected void toBytes(ByteBuffer buf) {
			super.toBytes(buf);
			buf.put(pattern);
		}
		
		public int getSize() {
			return typeSize + SYNC_PATTERN.length;
		}
	}

	public static class SendPacket extends TODMsg {
		public static final int MAX_DATA_LENGTH = 128;
		int seq;
		byte data[];
		
		public SendPacket() {
			type = Type.SendPacket;
		}
		
		public void toBytes(ByteBuffer buf) {
			assert(data.length <= MAX_DATA_LENGTH);
			
			super.toBytes(buf);
			buf.put((byte)data.length);
			buf.put((byte)((seq >> 0 ) & 0xFF));
			buf.put((byte)((seq >> 8 ) & 0xFF));
			buf.put((byte)((seq >> 16 ) & 0xFF));
			buf.put((byte)((seq >> 24 ) & 0xFF));
			buf.put(data);
		}
		
		public int getSize() {
			return typeSize + 4 + data.length;
		}
	}

	public static class SetChannel extends TODMsg {
		byte channel;
		
		public SetChannel() {
			type = Type.SetChannel;
		}
		
		public void toBytes(ByteBuffer buf) {
			super.toBytes(buf);
			buf.put(channel);
		}
		
		public int getSize() {
			return typeSize + 1;
		}
	}

	public void write(ByteBuffer buf) {
		toBytes(buf);
	}
	
	abstract public int getSize();
	
	protected void toBytes(ByteBuffer buf) {
		buf.put((byte)type.ordinal());
	}
}