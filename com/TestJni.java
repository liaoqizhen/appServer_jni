//package com.TestJni;

import android.util.Log;
import android.util.Slog;



public class TestJni {
	private static final String TAG = "testJni";

	static {
		System.loadLibrary("appServer_jni");
	}

	public native int videoHdmiInnerEnable(int channel, boolean enable);

	public static void main(String[] args) {
		TestJni obj = new TestJni();

		if(args.length<2) {
			System.out.println("Usage: TestJni channel enable");
			return;
		}

		int channel = Integer.parseInt(args[0]);
		boolean enable = (0 != Integer.parseInt(args[1]))?true:false;
		
		int result = obj.videoHdmiInnerEnable(channel, enable);	
		System.out.println("videoHdmiInnerEnable(" + channel + ", " + enable + ") = "    + result);
	}
}

