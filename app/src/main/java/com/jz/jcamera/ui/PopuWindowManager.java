package com.jz.jcamera.ui;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.drawable.BitmapDrawable;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.BaseAdapter;
import android.widget.LinearLayout;
import android.widget.ListView;
import android.widget.PopupWindow;

import com.jz.jcamera.R;
import com.jz.jcamera.util.JLog;

/**
 * @author jackzhous
 * @package com.jz.jcamera.ui
 * @filename PopuWindowManager
 * date on 2020/1/19 10:33 AM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class PopuWindowManager {

    private View parentView;
    private View contentView;
    private Context context;
    private PopupWindow window;
    private ListView listView;
    private String[] data;
    private onIntemListener listener;

    public void setListener(onIntemListener listener) {
        this.listener = listener;
    }

    public PopuWindowManager(View parentView, Context context) {
        this.parentView = parentView;
        this.context = context;
        contentView = LayoutInflater.from(context).inflate(R.layout.window_pop, null);
        window = new PopupWindow(contentView, LinearLayout.LayoutParams.WRAP_CONTENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        window.setSoftInputMode(WindowManager.LayoutParams.SOFT_INPUT_ADJUST_RESIZE);//必须加上这句，否则在有虚拟按键的华为手机上回出现遮盖住popowindows
        window.setTouchable(true);
        window.setOutsideTouchable(true);
        window.setBackgroundDrawable(new BitmapDrawable(context.getResources(),
                (Bitmap) null));
        initView();
    }


    private void initView(){

        listView = contentView.findViewById(R.id.lv);
        data = new String[]{"美颜", "边框", "马赛克", "多屏", "模糊", "无特效"};
        ArrayAdapter<String> adapter = new ArrayAdapter<>(context, R.layout.item_view
                , data);
        listView.setAdapter(adapter);
        listView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                if(listener != null){
                    listener.onClick(position+1);
                    dismiss();
                }
            }
        });
    }

    public void show(){
        window.showAsDropDown(parentView);
    }

    public void dismiss(){
        window.dismiss();
    }

    public interface onIntemListener {
        void onClick(int index);
    }

}
