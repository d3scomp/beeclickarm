package d3scomp.beeclickarmj;

public class CommException extends Exception {

	public CommException() {
	}

	public CommException(String message) {
		super(message);
	}

	public CommException(Throwable cause) {
		super(cause);
	}

	public CommException(String message, Throwable cause) {
		super(message, cause);
	}

	public CommException(String message, Throwable cause,
			boolean enableSuppression, boolean writableStackTrace) {
		super(message, cause, enableSuppression, writableStackTrace);
	}
}
