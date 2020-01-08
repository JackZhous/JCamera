package com.jz.jcamera.audio;

import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.os.Handler;
import android.os.Looper;

import com.jz.jcamera.util.JLog;

import java.nio.ByteBuffer;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

/**
 * @author jackzhous
 * @package com.jz.jcamera.audio
 * @filename AudioRecorder
 * date on 2019/12/25 11:13 AM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class AudioRecorder {

    private static final int SAMPLE_RATE = 44100;
    private int sampleFormat = AudioFormat.ENCODING_PCM_16BIT;
    private int channels = 1;

    private AudioRecord record;

    private AudioRecorderCallback callback;
    private Handler handler;

    private boolean isRecording;
    private int bufferSize;
    private ExecutorService thread;

    public boolean startRecord(){

        try {
            int channleLayout = channels == 1 ? AudioFormat.CHANNEL_IN_MONO : AudioFormat.CHANNEL_IN_STEREO;
            bufferSize = caluteBufferSize(channleLayout, sampleFormat);
            record = new AudioRecord(MediaRecorder.AudioSource.MIC, SAMPLE_RATE, channleLayout, sampleFormat, bufferSize);
        } catch (IllegalArgumentException e) {
            e.printStackTrace();
            JLog.i("create recorder failed");
            return false;
        }

        if(record.getState() != AudioRecord.STATE_INITIALIZED){
            JLog.i("init recorder failed");
            return false;
        }

        handler = new Handler(Looper.myLooper());
        thread = Executors.newCachedThreadPool();
        isRecording = true;
        thread.execute(this::recordRun);
        return true;
    }

    public int getChannels() {
        return channels;
    }
    /**
     * 录音工作线程
     */
    private void recordRun(){
        if(record.getState() != AudioRecord.STATE_INITIALIZED){
            return;
        }

        ByteBuffer buffer = ByteBuffer.allocate(bufferSize);
        record.startRecording();
        if(callback != null){
            handler.post(() -> callback.onAudioRecorderStarted());
        }
        int readSize = 0;
        while (isRecording){
            readSize = record.read(buffer.array(), 0, bufferSize);
            if(readSize > 0 && callback != null){
                byte[] data = new byte[readSize];
                buffer.position(0);
                buffer.limit(readSize);
                buffer.get(data, 0, readSize);
                handler.post(() -> callback.onAudioRecorderProgress(data));
            }
        }
        release();
        if(callback != null){
            callback.onAudioRecorderFinish();
        }
    }


    public void release(){
        if(record != null){
            record.stop();
            record.release();
            record = null;
        }
    }

    public void stop(){
        isRecording = false;
    }

    public int getSampleRate(){
        return sampleFormat;
    }


    private int caluteBufferSize(int channelLayout, int pcmFormat){
        int size = 1024;

        size = (channelLayout == AudioFormat.CHANNEL_IN_MONO) ? size : size << 1;

        size = (pcmFormat == AudioFormat.ENCODING_PCM_8BIT) ? size : size << 1;

        return size;
    }




    public interface AudioRecorderCallback{
        void onAudioRecorderStarted();

        void onAudioRecorderProgress(byte[] data);

        void onAudioRecorderFinish();
    }

    public void setCallback(AudioRecorderCallback callback) {
        this.callback = callback;
    }
}
