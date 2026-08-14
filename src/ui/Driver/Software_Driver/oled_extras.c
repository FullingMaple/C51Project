/* oled_extras.c —— 未使用的 3D 演示代码（不参与编译，未来启用）*/
#include "OLED.h"

code int edges[12][2] = {
    {0,1},{1,2},{2,3},{3,0},
    {4,5},{5,6},{6,7},{7,4},
    {0,4},{1,5},{2,6},{3,7}
};

// ==== 主绘制函数 ====
void DrawCube3D(int16_t centerX, int16_t centerY, float size, uint8_t perspective) {
    // 三角函数计算
    float sinY = sin(yaw), cosY = cos(yaw);
    float sinP = sin(pitch), cosP = cos(pitch);
    float sinR = sin(roll), cosR = cos(roll);

    const float cube[8][3] = {
        {-1,-1,-1},{1,-1,-1},{1,1,-1},{-1,1,-1},
        {-1,-1,1},{1,-1,1},{1,1,1},{-1,1,1}
    };

//    float rotated[8][3];
    int projected[8][2];

    for (int i = 0; i < 8; i++) {
        float x = cube[i][0] * size / 2;
        float y = cube[i][1] * size / 2;
        float z = cube[i][2] * size / 2;

        // Roll (X-axis)
        float y1 = cosR * y - sinR * z;
        float z1 = sinR * y + cosR * z;
        y = y1; z = z1;
        // Pitch (Y-axis)
        float x1 = cosP * x + sinP * z;
        z1 = -sinP * x + cosP * z;
        x = x1; z = z1;
        // Yaw (Z-axis)
        x1 = cosY * x - sinY * y;
        y1 = sinY * x + cosY * y;
        x = x1; y = y1;

//        rotated[i][0] = x;
//        rotated[i][1] = y;
//        rotated[i][2] = z;

        // 投影
        if (perspective) {
            float distance = 50.0f;
            float factor = distance / (distance + z);
            projected[i][0] = (int)(x * factor) + centerX;
            projected[i][1] = (int)(y * factor) + centerY;
        } else {
            projected[i][0] = (int)x + centerX;
            projected[i][1] = (int)y + centerY;
        }
    }

    // 清屏并画线
    //OLED_Clear();
    for (int i = 0; i < 12; i++) {
        int a = edges[i][0], b = edges[i][1];
        OLED_DrawLine(projected[a][0], projected[a][1], projected[b][0], projected[b][1]);
    }
    //OLED_Update();

    // 更新欧拉角（旋转速度）
    yaw   += DEG2RAD(yaw_v);
    pitch += DEG2RAD(pitch_v);
    roll  += DEG2RAD(roll_v);

    // 保证角度在 [0, 2PI) 内循环
    if (yaw > 6.283f) yaw -= 6.283f;
    if (pitch > 6.283f) pitch -= 6.283f;
    if (roll > 6.283f) roll -= 6.283f;
}

