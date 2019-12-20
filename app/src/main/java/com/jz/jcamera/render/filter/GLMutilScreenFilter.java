package com.jz.jcamera.render.filter;

import android.content.Context;

import com.jz.jcamera.R;
import com.jz.jcamera.render.filter.BaseFilter;
import com.jz.jcamera.util.OpenGLUtil;

/**
 * @author jackzhous
 * @package com.jz.jcamera.render
 * @filename GLMutilScreenFilter
 * date on 2019/12/16 10:57 AM
 * @describe TODO
 * @email jackzhouyu@foxmail.com
 **/
public class GLMutilScreenFilter extends BaseFilter {
    public GLMutilScreenFilter(Context context) {
        this(context, VERTEX_SHADER, OpenGLUtil.readRawFile(context, R.raw.fragment_multiple_screen));
    }

    public GLMutilScreenFilter(Context context, String vertexShader, String fragmentShader) {
        super(context, vertexShader, fragmentShader);
    }


}
