# 作业回顾与思考

学而不思则罔 思而不学则殆。

下面涉及剧透，谨慎查看。


## 作业0 & 作业1
  * 没有比较特殊的问题。

## 作业2
> 图示: 图1没有使用MSAA,有明显锯齿；图2错误的MSAA，未记录子像素颜色和深度，有黑边；图3, MSAA, 锯齿缓解，无黑边。
![没有supersampling](./hw2.png "没有supersampling"){:height="25%" width="25%"} ![MSAA但有黑边](./hw2withblackbounder.png "MSAA但有黑边"){:height="25%" width="25%"} ![MSAA正常](./hw2massright.png "正常情况"){:height="25%" width="25%"}


1. 画出来的两个三角形堆叠先后与效果图相反?
   * 深度测试的问题。在作业1中的**get_projection_matrix函数**中深度是**负数**, 负数越小说明越接近屏幕。
   * 在**rasterizer.cpp**中经过viewport transformation后，原本距离屏幕更近点的Z坐标值却越大。
   * 经过转换后的Z值会在**rasterize_triangle函数**中被视作距离，也即距离屏幕越近的点，Z坐标值应越小。至此，绘制出的效果图中会出现离屏幕越远的点覆盖越近的点。
   * Z值相反，则在**get_projection_matrix**函数中，对Z做一次Z轴对称的变换即可。


2. MSAA有黑边?
    * 黑边形成的原因与具体实现有关。
      * 没有保存次像素的深度和颜色, 图形边界更新像素点的颜色只与靠前的图形颜色相关，并且会取颜色的百分比，边界会暗一些。
      * 保存了次像素的深度和颜色，但是目标像素的**更新方式**有变，主要区别为MSAA更新像素值不用再检查深度, 代码如下：
  ```c++
    auto update_depth_and_color = [this](int x, int y, float test_depth, const Vector3f& color, bool update_color_with_depth_check=true){
        int index = get_index(x, y);
        if(test_depth < depth_buf[index])
        {
            depth_buf[index] = test_depth;
            if(update_color_with_depth_check)
                set_pixel({x * 1.f, y * 1.f, test_depth}, color);
        }
        if(!update_color_with_depth_check)
        {
            set_pixel({x * 1.f, y * 1.f, test_depth}, color);
        }
    };
  ```
