/***
	************************************************************************************************
	*	@version V1.0
	*  @date    2024-4-19
	*	@author  反客科技
   ************************************************************************************************
   *  @description
	*
	*	实验平台：反客STM32H750XBH6核心板 （型号：FK750M6-XBH6）
	*	淘宝地址：https://shop212360197.taobao.com
	*	QQ交流群：536665479
	*
>>>>> 文件说明：
	*
	*  SDRAM相关初始化函数
	*
	************************************************************************************************
***/

#include "sdram.h"
#include <stdio.h>

/******************************************************************************************************
 *	函 数 名: SDRAM_Initialization_Sequence
 *	入口参数: hsdram - SDRAM_HandleTypeDef定义的变量，即表示定义的sdram
 *	返 回 值: 无
 *	函数功能: SDRAM 参数配置
 *	说    明: 配置SDRAM相关时序和控制方式
 *******************************************************************************************************/

void SDRAM_Initialization_Sequence(SDRAM_HandleTypeDef *hsdram)
{
	__IO uint32_t tmpmrd = 0;
	FMC_SDRAM_CommandTypeDef Command; // 控制指令

	/* Configure a clock configuration enable command */
	Command.CommandMode = FMC_SDRAM_CMD_CLK_ENABLE;	 // 开启SDRAM时钟
	Command.CommandTarget = FMC_COMMAND_TARGET_BANK; // 选择要控制的区域
	Command.AutoRefreshNumber = 1;
	Command.ModeRegisterDefinition = 0;

	HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT); // 发送控制指令
	HAL_Delay(1);											// 延时等待

	/* Configure a PALL (precharge all) command */
	Command.CommandMode = FMC_SDRAM_CMD_PALL;		 // 预充电命令
	Command.CommandTarget = FMC_COMMAND_TARGET_BANK; // 选择要控制的区域
	Command.AutoRefreshNumber = 1;
	Command.ModeRegisterDefinition = 0;

	HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT); // 发送控制指令

	/* Configure a Auto-Refresh command */
	Command.CommandMode = FMC_SDRAM_CMD_AUTOREFRESH_MODE; // 使用自动刷新
	Command.CommandTarget = FMC_COMMAND_TARGET_BANK;	  // 选择要控制的区域
	Command.AutoRefreshNumber = 8;						  // 自动刷新次数
	Command.ModeRegisterDefinition = 0;

	HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT); // 发送控制指令

	/* Program the external memory mode register */
	tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1 |
			 SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL |
			 SDRAM_MODEREG_CAS_LATENCY_3 |
			 SDRAM_MODEREG_OPERATING_MODE_STANDARD |
			 SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	Command.CommandMode = FMC_SDRAM_CMD_LOAD_MODE;	 // 加载模式寄存器命令
	Command.CommandTarget = FMC_COMMAND_TARGET_BANK; // 选择要控制的区域
	Command.AutoRefreshNumber = 1;
	Command.ModeRegisterDefinition = tmpmrd;

	HAL_SDRAM_SendCommand(hsdram, &Command, SDRAM_TIMEOUT); // 发送控制指令

	HAL_SDRAM_ProgramRefreshRate(hsdram, 1543); // 配置刷新率
}

// #define SDRAM_Size 32*1024*1024  //32M字节

/******************************************************************************************************
 *	函 数 名: SDRAM_Test
 *	入口参数: 无
 *	返 回 值: SUCCESS - 成功，ERROR - 失败
 *	函数功能: SDRAM测试
 *	说    明: 先以16位的数据宽度写入数据，再读取出来一一进行比较，随后以8位的数据宽度写入，
 *				 用以验证NBL0和NBL1两个引脚的连接是否正常。
 *******************************************************************************************************/

uint8_t SDRAM_Test(uint32_t BaseAddr, uint32_t size)
{
	uint32_t i = 0; // 计数变量
	__IO uint32_t *pSDRAM_32b;
	__IO uint16_t *pSDRAM_16b;
	__IO uint8_t *pSDRAM_8b;
	uint32_t ReadData_32b = 0; // 读取到的数据
	uint16_t ReadData_16b = 0;
	uint8_t ReadData_8b = 0;

	uint32_t ExecutionTime_Begin; // 开始时间
	uint32_t ExecutionTime_End;	  // 结束时间
	uint32_t ExecutionTime;		  // 执行时间
	float ExecutionSpeed;		  // 执行速度

	printf("\r\n***********************************************************************************************\r\n");
	printf("\r\n进行速度测试>>>\r\n\r\n");

	// 写入 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	pSDRAM_32b = (uint32_t *)BaseAddr;
	printf("以32位数据宽度写入...\r\n");
	ExecutionTime_Begin = HAL_GetTick(); // 获取 systick 当前时间，单位ms
	for (i = 0; i < size / 4; i++)
	{
		*pSDRAM_32b++ = i; // 写入数据
	}
	ExecutionTime_End = HAL_GetTick();										 // 获取 systick 当前时间，单位ms
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;				 // 计算擦除时间，单位ms
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("写入数据完毕，大小：%ld MB，耗时: %ld ms, 写入速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	// 读取	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	pSDRAM_32b = (uint32_t *)BaseAddr;
	ExecutionTime_Begin = HAL_GetTick(); // 获取 systick 当前时间，单位ms
	for (i = 0; i < size / 4; i++)
	{
		ReadData_32b = *pSDRAM_32b++; // 从SDRAM读出数据
	}
	ExecutionTime_End = HAL_GetTick();										 // 获取 systick 当前时间，单位ms
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;				 // 计算擦除时间，单位ms
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("读取数据完毕，大小：%ld MB，耗时: %ld ms, 读取速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	//// 数据校验 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	pSDRAM_32b = (uint32_t *)BaseAddr;
	printf("进行数据校验...\r\n");
	for (i = 0; i < size / 4; i++)
	{
		ReadData_32b = *pSDRAM_32b++; // 从SDRAM读出数据
		if (ReadData_32b != i)		   // 检测数据，若不相等，跳出函数,返回检测失败结果。
		{
			printf("\r\nSDRAM测试失败！！出错位置：%ld,读出数据：%ld\r\n ", i, ReadData_32b);
			return ERROR; // 返回失败标志
		}
	}

	printf("32位数据宽度读写通过。\r\n\r\n以16位数据宽度写入...\r\n");
	pSDRAM_16b = (uint16_t *)BaseAddr;
	ExecutionTime_Begin = HAL_GetTick();
	for (i = 0; i < size / 2; i++)
	{
		*pSDRAM_16b++ = (uint16_t)i;
	}
	ExecutionTime_End = HAL_GetTick();
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("写入数据完毕，大小：%ld MB，耗时: %ld ms, 写入速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	// 读取	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	pSDRAM_16b = (uint16_t *)BaseAddr;
	ExecutionTime_Begin = HAL_GetTick(); // 获取 systick 当前时间，单位ms
	for (i = 0; i < size / 2; i++)
	{
		ReadData_16b = *pSDRAM_16b++; // 从SDRAM读出数据
	}
	ExecutionTime_End = HAL_GetTick();										 // 获取 systick 当前时间，单位ms
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;				 // 计算擦除时间，单位ms
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("读取数据完毕，大小：%ld MB，耗时: %ld ms, 读取速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	//// 数据校验 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	printf("进行数据校验...\r\n");
	pSDRAM_16b = (uint16_t *)BaseAddr;
	for (i = 0; i < size / 2; i++)
	{
		ReadData_16b = *pSDRAM_16b++;
		if (ReadData_16b != (uint16_t)i) // 检测数据，若不相等，跳出函数,返回检测失败结果。
		{
			printf("\r\n16位数据宽度读写测试失败！！\r\n");
			printf("请检查NBL0和NBL1的连接\r\n");
			return ERROR; // 返回失败标志
		}
	}

	printf("16位数据宽度读写通过。\r\n\r\n以8位数据宽度写入...\r\n");
	pSDRAM_8b = (uint8_t *)BaseAddr;
	ExecutionTime_Begin = HAL_GetTick();
	for (i = 0; i < size; i++)
	{
		*pSDRAM_8b++ = (uint8_t)i;
	}
	ExecutionTime_End = HAL_GetTick();
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("写入数据完毕，大小：%ld MB，耗时: %ld ms, 写入速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	// 读取	>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	pSDRAM_8b = (uint8_t *)BaseAddr;
	ExecutionTime_Begin = HAL_GetTick(); // 获取 systick 当前时间，单位ms
	for (i = 0; i < size; i++)
	{
		ReadData_8b = *pSDRAM_8b++; // 从SDRAM读出数据
	}
	ExecutionTime_End = HAL_GetTick();										 // 获取 systick 当前时间，单位ms
	ExecutionTime = ExecutionTime_End - ExecutionTime_Begin;				 // 计算擦除时间，单位ms
	ExecutionSpeed = (float)size / 1024 / 1024 / ExecutionTime * 1000; // 计算速度，单位 MB/S
	printf("读取数据完毕，大小：%ld MB，耗时: %ld ms, 读取速度：%.2f MB/s\r\n", size / 1024 / 1024, ExecutionTime, ExecutionSpeed);

	//// 数据校验 >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
	printf("进行数据校验...\r\n");
	pSDRAM_8b = (uint8_t *)BaseAddr;
	for (i = 0; i < size; i++)
	{
		ReadData_8b = *pSDRAM_8b++;
		if (ReadData_8b != (uint8_t)i) // 检测数据，若不相等，跳出函数,返回检测失败结果。
		{
			printf("\r\n8位数据宽度读写测试失败！！\r\n");
			printf("请检查NBL0和NBL1的连接\r\n");
			return ERROR; // 返回失败标志
		}
	}

	printf("8位数据宽度读写通过。\r\n\r\nSDRAM读写测试通过，系统正常\r\n");
	printf("\r\n***********************************************************************************************\r\n");

	return SUCCESS; // 返回成功标志
}
