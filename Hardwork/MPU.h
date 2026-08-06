#ifndef _MPU6050_H
#define _MPU6050_H

typedef struct 
{
    float q0, q1, q2, q3;
} Quaternion;
 
void MPU6050_WriteReg(uint8_t RegAddress,uint8_t Data);
uint8_t MPU6050_ReadReg(uint8_t RegAddress);
void MPU6050_Init();
void MPU6050_Calibration(void);
void MPU6050_GetData(int16_t *AccX,int16_t *AccY,int16_t *AccZ,int16_t *GyroX,int16_t *GyroY,int16_t *GyroZ);
void MPU6050_ReadSensors(float *ax, float *ay, float *az, float *gx, float *gy, float *gz);
void NormalizeAccel(float *ax, float *ay, float *az);
void ComputeError(float ax, float ay, float az, float *error_x, float *error_y, float *error_z);
void UpdateQuaternion(float gx, float gy, float gz, float error_x, float error_y, float error_z, float dt);
void ComputeEulerAngles(float *pitch, float *roll, float *yaw);
void GetAngles(float *pitch, float *roll, float *yaw, float dt);
#endif

