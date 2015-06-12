package d3scomp.beeclickarmj;

import java.nio.ByteBuffer;

abstract class TODMsg {
	static final int typeSize = 1;
	
	Type type;
	
	static enum Type {
		SYNC, SEND_PACKET, SET_CHANNEL, SET_TXPOWER, SET_ADDR
	}
	
	static class Sync extends TODMsg {
		static final byte[] SYNC_PATTERN = {'H', 'e', 'l', 'l', 'o', ' ', 't', 'h', 'e', 'r', 'e', '!'};
		
		Sync() {
			type = Type.SYNC;
		}
		
		protected void toBytes(ByteBuffer buf) {
			super.toBytes(buf);
			buf.put(SYNC_PATTERN);
		}
		
		int getSize() {
			return typeSize + SYNC_PATTERN.length;
		}
	}

	static class SendPacket extends TODMsg {
		static final int MAX_DATA_LENGTH = 128;
		int seq;
		byte data[];
		
		SendPacket() {
			type = Type.SEND_PACKET;
		}
		
		protected void toBytes(ByteBuffer buf) {
			assert(data.length <= MAX_DATA_LENGTH);
			
			super.toBytes(buf);
			buf.put((byte)data.length);
			buf.put((byte)((seq >> 0 ) & 0xFF));
			buf.put((byte)((seq >> 8 ) & 0xFF));
			buf.put((byte)((seq >> 16 ) & 0xFF));
			buf.put((byte)((seq >> 24 ) & 0xFF));
			buf.put(data);
		}
		
		int getSize() {
			return typeSize + 5 + data.length;
		}
	}

	static class SetChannel extends TODMsg {
		int channel;
		
		SetChannel() {
			type = Type.SET_CHANNEL;
		}
		
		protected void toBytes(ByteBuffer buf) {
			super.toBytes(buf);
			buf.put((byte)channel);
		}
		
		int getSize() {
			return typeSize + 1;
		}
	}
	
	static class SetPower extends TODMsg {
		short power;
		
		SetPower() {
			type = Type.SET_TXPOWER;
		}
		
		protected void toBytes(ByteBuffer buf) {
			super.toBytes(buf);
			buf.putShort(Short.reverseBytes(power));
		}
		
		int getSize() {
			return typeSize + 2;
		}
	}

	static class SetAddr extends TODMsg {
		int panId;
		int sAddr;
		
		SetAddr() {
			type = Type.SET_ADDR;
		}
		
		protected void toBytes(ByteBuffer buf) {
			super.toBytes(buf);
			buf.put((byte)((panId >> 0 ) & 0xFF));
			buf.put((byte)((panId >> 8 ) & 0xFF));
			buf.put((byte)((sAddr >> 0 ) & 0xFF));
			buf.put((byte)((sAddr >> 8 ) & 0xFF));
		}
		
		int getSize() {
			return typeSize + 4;
		}
	}
	
	void write(ByteBuffer buf) {
		toBytes(buf);
	}
	
	abstract int getSize();
	
	protected void toBytes(ByteBuffer buf) {
		buf.put((byte)type.ordinal());
	}
}