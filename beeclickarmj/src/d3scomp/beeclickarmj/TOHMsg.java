package d3scomp.beeclickarmj;

import java.nio.ByteBuffer;

abstract class TOHMsg {
	static final int MAX_MSG_SIZE = 1024;
	
	static final int typeSize = 1;
	
	Type type;
	
	static enum Type {
		SYNC, RECV_PACKET, PACKET_SENT, CHANNEL_SET, ADDR_SET, INFO
	}
	
	protected static interface TOHMsgFactory {
		TOHMsg newInstance();
		int getExpectedSize(ByteBuffer buf);
	}
	
	protected static TOHMsgFactory[] msgFactories = {
		null, // SYNC is handled separately
		new TOHMsgFactory() {
			public TOHMsg newInstance() {
				return new RecvPacket();
			}
			public int getExpectedSize(ByteBuffer buf) {
				return RecvPacket.getExpectedSizeLowerBound(buf);
			}
		},
		new TOHMsgFactory() {
			public TOHMsg newInstance() {
				return new PacketSent();
			}
			public int getExpectedSize(ByteBuffer buf) {
				return PacketSent.getExpectedSizeLowerBound(buf);
			}
		},
		new TOHMsgFactory() {
			public TOHMsg newInstance() {
				return new ChannelSet();
			}
			public int getExpectedSize(ByteBuffer buf) {
				return ChannelSet.getExpectedSizeLowerBound(buf);
			}
		},
		new TOHMsgFactory() {
			public TOHMsg newInstance() {
				return new AddrSet();
			}
			public int getExpectedSize(ByteBuffer buf) {
				return AddrSet.getExpectedSizeLowerBound(buf);
			}
		},
		new TOHMsgFactory() {
			public TOHMsg newInstance() {
				return new Info();
			}
			public int getExpectedSize(ByteBuffer buf) {
				return Info.getExpectedSizeLowerBound(buf);
			}
		}		
	};
	
	static class Sync extends TOHMsg {
		static final byte[] SYNC_PATTERN = TODMsg.Sync.SYNC_PATTERN;
		
		static final byte[] correctSyncBytes;
		static {
			correctSyncBytes = new byte[1 + SYNC_PATTERN.length];
			correctSyncBytes[0] = (byte)Type.SYNC.ordinal();
			System.arraycopy(SYNC_PATTERN, 0, correctSyncBytes, 1, SYNC_PATTERN.length);
		}

		Sync() {
			type = Type.SYNC;
		}
		
		protected void fromBytes(ByteBuffer buf) {
			assert(false); // SYNC is handled separately
		}
	}

	static class RecvPacket extends TOHMsg {
		static final int MAX_DATA_LENGTH = 128;
		int rssi;
		int lqi;
		byte fcs;
		byte data[];
		
		RecvPacket() {
			type = Type.RECV_PACKET;
		}
		
		protected void fromBytes(ByteBuffer buf) {
			super.fromBytes(buf);
			
			int length = buf.get() & 0xFF;
			assert(length <= MAX_DATA_LENGTH);
			
			rssi = buf.get() & 0xFF;
			lqi = buf.get() & 0xFF;
			fcs = buf.get();
			
			data = new byte[length];
			buf.get(data);
		}
		
		static int getExpectedSizeLowerBound(ByteBuffer buf) {
			return typeSize + 4 + (buf.position() == 1 ? 0 : (buf.get(1) & 0xFF));
		}
	}

	static class PacketSent extends TOHMsg {
		int seq;
		int status; // 0 - OK, 1 - error
		
		PacketSent() {
			type = Type.PACKET_SENT;
		}
		
		protected void fromBytes(ByteBuffer buf) {
			super.fromBytes(buf);
			seq = ((buf.get() & 0xFF) << 0) | ((buf.get() & 0xFF) << 8) | ((buf.get() & 0xFF) << 16) | ((buf.get() & 0xFF) << 24);
			status = buf.get() & 0xFF;
		}
		
		static int getExpectedSizeLowerBound(ByteBuffer buf) {
			return typeSize + 4 + 1;
		}
	}

	static class ChannelSet extends TOHMsg {
		int channel;
		
		ChannelSet() {
			type = Type.CHANNEL_SET;
		}
		
		protected void fromBytes(ByteBuffer buf) {
			super.fromBytes(buf);
			channel = buf.get();
		}
		
		static int getExpectedSizeLowerBound(ByteBuffer buf) {
			return typeSize + 1;
		}
	}

	static class AddrSet extends TOHMsg {
		int panId;
		int sAddr;
		
		AddrSet() {
			type = Type.ADDR_SET;
		}
		
		protected void fromBytes(ByteBuffer buf) {
			super.fromBytes(buf);
			panId = ((buf.get() & 0xFF) << 0) | ((buf.get() & 0xFF) << 8);
			sAddr = ((buf.get() & 0xFF) << 0) | ((buf.get() & 0xFF) << 8);
		}
		
		static int getExpectedSizeLowerBound(ByteBuffer buf) {
			return typeSize + 4;
		}
	}

	static class Info extends TOHMsg {
		static final int MAX_INFO_TEXT_LENGTH = 255;
		String text;
		
		Info() {
			type = Type.INFO;
		}
		
		protected void fromBytes(ByteBuffer buf) {
			super.fromBytes(buf);
			
			int length = buf.get() & 0xFF;
			assert(length <= MAX_INFO_TEXT_LENGTH);
			
			byte[] data = new byte[length];
			buf.get(data);
			
			text = new String(data);
		}
		
		static int getExpectedSizeLowerBound(ByteBuffer buf) {
			return typeSize + 1 + (buf.position() == 1 ? 0 : (buf.get(1) & 0xFF));
		}
	}

	static TOHMsg read(ByteBuffer buf) {
		TOHMsg msg = msgFactories[buf.get(0)].newInstance();
		msg.fromBytes(buf);
		
		return msg;
	}
	
	static int getExpectedSizeLowerBound(ByteBuffer buf) {
		return msgFactories[buf.get(0)].getExpectedSize(buf);
	}
	
	protected void fromBytes(ByteBuffer buf) {
		byte msgType = buf.get();
		
		assert(msgType == type.ordinal());
	}
}