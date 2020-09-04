USE [9lottery]
GO

/****** Object:  StoredProcedure [dbo].[sp_GenerateGameNumberUpdateTrigger]    Script Date: 09/04/2020 16:35:45 ******/
IF  EXISTS (SELECT * FROM sys.objects WHERE object_id = OBJECT_ID(N'[dbo].[sp_GenerateGameNumberUpdateTrigger]') AND type in (N'P', N'PC'))
DROP PROCEDURE [dbo].[sp_GenerateGameNumberUpdateTrigger]
GO

USE [9lottery]
GO

/****** Object:  StoredProcedure [dbo].[sp_GenerateGameNumberUpdateTrigger]    Script Date: 09/04/2020 16:35:45 ******/
SET ANSI_NULLS ON
GO

SET QUOTED_IDENTIFIER ON
GO







CREATE PROCEDURE [dbo].[sp_GenerateGameNumberUpdateTrigger]
AS
BEGIN
	--控制开关未开启，不控制
	if (select ISNULL(GameUpdateNumberOpen,0) from 9lottery.dbo.tab_GameNumberSet) = 0
		return

	--获取控制信息
	declare @UserControled int = 0
	declare @ControlRate int = 0
	declare @PeriodGap int = 0
	declare @PowerControl int = 0
	select top 1 @UserControled = UserControled, @ControlRate = ControlRate, @PeriodGap = ISNULL(PeriodGap, 100), @PowerControl = PowerControl
		from 9lottery.dbo.tab_Game_Control order by UpdateTime desc
	if @PowerControl > 2
		set @PowerControl = 2
	else if @PowerControl < 0
		set @PowerControl = 0
	print '受控用户@UserControled:' + cast(@UserControled as varchar(10))
	print '控制指数@ControlRate:' + cast(@ControlRate as varchar(10))
	print '选取期数区间@PeriodGap:' + cast(@PeriodGap as varchar(10))
	print '强弱控制@PowerControl:' + cast(@PowerControl as varchar(10))
	
	--获取当前期号 时间段选择在一分钟之内的
	declare @CurrentTime datetime = getdate()
	declare @CurrentTimeLastMin datetime = dateadd(MINUTE,-1,convert(CHAR(16),'2020-09-03 23:30:19.000',120)+':00.000')
	declare @Counts int = 0
	declare @LoopCounts int = 0
	declare @OptState int = -1
	declare @IntervalM int = 0
	declare @CurrentIssueNumber varchar(60) = ''
	declare @BeginIssueNumber varchar(30) = ''
	declare @LastIssueNumber varchar(30) = ''
	
	select @Counts = count(*) from 9lottery.dbo.tab_Games_Awei where State=0 and StartTime>@CurrentTimeLastMin and StartTime<='2020-09-03 23:30:20.000'
	while @LoopCounts < @Counts
	begin
		set @LoopCounts = @LoopCounts + 1
		select @CurrentIssueNumber=IssueNumber, @OptState=OptState, @IntervalM = IntervalM from  
			(select row_number() over(order by TypeID) as rowid, * from 9lottery.dbo.tab_Games_Awei where State=0 
				and StartTime>@CurrentTimeLastMin and StartTime<='2020-09-03 23:30:20.000') as t 
			where rowid=@LoopCounts
		
		set @LastIssueNumber = cast((cast(@CurrentIssueNumber as bigint)-1) as varchar(30))
		--计算@BeginIssueNumber
		declare @VarDay int = cast(substring(@CurrentIssueNumber, 0, 9) as int)
		declare @TypeID varchar(2) = substring(@CurrentIssueNumber, 9, 1)
		declare @Period int = cast(substring(@CurrentIssueNumber, 10, 4) as int)
		if @PeriodGap <= @Period
		begin 
			declare @TmpNumber bigint = cast(@CurrentIssueNumber as bigint) - @PeriodGap
			set @BeginIssueNumber = cast(@TmpNumber as varchar(30))
		end
		else if @PeriodGap > @Period
		begin 
			declare @BaseNum int = 24*60/@IntervalM
			declare @PeriodNotInToday int = @PeriodGap - @Period
			declare @DayNums int = 0 - (@PeriodNotInToday / @BaseNum + 1)
			declare @PeriodNums int = @BaseNum - @PeriodNotInToday % @BaseNum
			declare @TargetDate datetime = dateadd(Day, @DayNums, CONVERT(varchar(12), getdate(), 112))
			declare @VarTargetDate varchar(30) = CONVERT(varchar(12), @TargetDate, 112)
			if len(@PeriodNums) = 1
				set @BeginIssueNumber = @VarTargetDate + @TypeID + '000' + cast(@PeriodNums as varchar(10))
			else if len(@PeriodNums) = 2
				set @BeginIssueNumber = @VarTargetDate + @TypeID +'00' + cast(@PeriodNums as varchar(10))
			else if len(@PeriodNums) = 3
				set @BeginIssueNumber = @VarTargetDate + @TypeID +'0' + cast(@PeriodNums as varchar(10))
			else
				set @BeginIssueNumber = @VarTargetDate + @TypeID +cast(@PeriodNums as varchar(10))
		end
		print '起始期数@BeginIssueNumber:' + @BeginIssueNumber 
		print '上一期数@LastIssueNumber:' + @LastIssueNumber 

		
		print '调用存储过程处理@LoopCounts:' + cast(@LoopCounts as varchar(10))
	end
END

















GO


