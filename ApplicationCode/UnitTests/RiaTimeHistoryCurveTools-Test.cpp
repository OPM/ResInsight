#include "gtest/gtest.h"

#include "RiaTimeHistoryCurveResampler.h"
#include "RiaQDateTimeTools.h"


//--------------------------------------------------------------------------------------------------
/// Constants
//--------------------------------------------------------------------------------------------------
static int SECS_PER_DAY = 60 * 60 * 24;

//--------------------------------------------------------------------------------------------------
/// Helpers
//--------------------------------------------------------------------------------------------------
static time_t toTime_t(const QString& timeString)
{
    return RiaQDateTimeTools::fromString(timeString, "yyyy-MM-dd").toTime_t();
}

static std::vector<time_t> toTime_tVector(const std::vector<QString>& timeStrings)
{
    std::vector<time_t> tv;
    for (auto& ts : timeStrings) tv.push_back(toTime_t(ts));
    return tv;
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_Resampling_NoPeriod)
{
    std::vector<QString> timeStrings(
        {
            "2018-02-03",
            "2018-02-27"
        }
    );

    std::vector<double> dataValues(
        {
            3.0,
            5.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::MONTH);

    EXPECT_EQ(1, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps()[0]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_Resampling_Decade)
{
    std::vector<QString> timeStrings(
        {
            "1989-02-03",
            "2005-06-06",
            "2012-02-07"
        }
    );

    std::vector<double> dataValues(
        {
            0.0,
            0.0,
            0.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::DECADE);

    EXPECT_EQ(4, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("1990-01-01"), resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(toTime_t("2000-01-01"), resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(toTime_t("2010-01-01"), resampler.resampledTimeSteps()[2]);
    EXPECT_EQ(toTime_t("2020-01-01"), resampler.resampledTimeSteps()[3]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_Resampling_Year)
{
    std::vector<QString> timeStrings(
        {
            "2014-06-06",
            "2015-12-02",
            "2018-02-07"
        }
    );

    std::vector<double> dataValues(
        {
            0.0,
            0.0,
            0.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::YEAR);

    EXPECT_EQ(5, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2015-01-01"), resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(toTime_t("2016-01-01"), resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(toTime_t("2017-01-01"), resampler.resampledTimeSteps()[2]);
    EXPECT_EQ(toTime_t("2018-01-01"), resampler.resampledTimeSteps()[3]);
    EXPECT_EQ(toTime_t("2019-01-01"), resampler.resampledTimeSteps()[4]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_Resampling_HalfYear)
{
    std::vector<QString> timeStrings(
        {
            "2016-06-06",
            "2017-03-02",
            "2018-02-07"
        }
    );

    std::vector<double> dataValues(
        {
            0.0,
            0.0,
            0.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::HALFYEAR);

    EXPECT_EQ(5, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2016-07-01"), resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(toTime_t("2017-01-01"), resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(toTime_t("2017-07-01"), resampler.resampledTimeSteps()[2]);
    EXPECT_EQ(toTime_t("2018-01-01"), resampler.resampledTimeSteps()[3]);
    EXPECT_EQ(toTime_t("2018-07-01"), resampler.resampledTimeSteps()[4]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_Resampling_Quarter)
{
    std::vector<QString> timeStrings(
        {
            "2016-09-06",
            "2017-03-02",
            "2018-02-07"
        }
    );

    std::vector<double> dataValues(
        {
            0.0,
            0.0,
            0.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::QUARTER);

    EXPECT_EQ(7, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2016-10-01"), resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(toTime_t("2017-01-01"), resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(toTime_t("2017-04-01"), resampler.resampledTimeSteps()[2]);
    EXPECT_EQ(toTime_t("2017-07-01"), resampler.resampledTimeSteps()[3]);
    EXPECT_EQ(toTime_t("2017-10-01"), resampler.resampledTimeSteps()[4]);
    EXPECT_EQ(toTime_t("2018-01-01"), resampler.resampledTimeSteps()[5]);
    EXPECT_EQ(toTime_t("2018-04-01"), resampler.resampledTimeSteps()[6]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_Resampling_Month)
{
    std::vector<QString> timeStrings(
        {
            "2017-09-06",
            "2017-12-02",
            "2018-02-07"
        }
    );

    std::vector<double> dataValues(
        {
            0.0,
            0.0,
            0.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::MONTH);

    EXPECT_EQ(6, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2017-10-01"), resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(toTime_t("2017-11-01"), resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(toTime_t("2017-12-01"), resampler.resampledTimeSteps()[2]);
    EXPECT_EQ(toTime_t("2018-01-01"), resampler.resampledTimeSteps()[3]);
    EXPECT_EQ(toTime_t("2018-02-01"), resampler.resampledTimeSteps()[4]);
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps()[5]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_Resampling_Week)
{
    std::vector<QString> timeStrings(
        {
            "2017-11-02",
            "2017-12-24",
            "2018-01-07"
        }
    );

    std::vector<double> dataValues(
        {
            0.0,
            0.0,
            0.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::WEEK);

    EXPECT_EQ(10, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2017-11-06"), resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(toTime_t("2017-11-13"), resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(toTime_t("2017-11-20"), resampler.resampledTimeSteps()[2]);
    EXPECT_EQ(toTime_t("2017-11-27"), resampler.resampledTimeSteps()[3]);
    EXPECT_EQ(toTime_t("2017-12-04"), resampler.resampledTimeSteps()[4]);
    EXPECT_EQ(toTime_t("2017-12-11"), resampler.resampledTimeSteps()[5]);
    EXPECT_EQ(toTime_t("2017-12-18"), resampler.resampledTimeSteps()[6]);
    EXPECT_EQ(toTime_t("2017-12-25"), resampler.resampledTimeSteps()[7]);
    EXPECT_EQ(toTime_t("2018-01-01"), resampler.resampledTimeSteps()[8]);
    EXPECT_EQ(toTime_t("2018-01-08"), resampler.resampledTimeSteps()[9]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_Resampling_NoSampleCrossingPeriodBoundary)
{
    std::vector<QString> timeStrings(
        {
            "2017-01-02",
            "2017-06-15",
            "2017-12-24"
        }
    );

    std::vector<double> dataValues(
        {
            0.0,
            0.0,
            0.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::YEAR);

    EXPECT_EQ(1, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-01-01"), resampler.resampledTimeSteps()[0]);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_WeightedMean_SingleSample)
{
    std::vector<QString> timeStrings(
        {
            "2018-02-07"
        }
    );

    std::vector<double> dataValues(
        {
            3.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::MONTH);

    EXPECT_EQ(1, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps()[0]);

    double value = 0.0;
    EXPECT_NEAR(value, resampler.resampledValues()[0], 1e-12);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_WeightedMean_Days)
{
    std::vector<QString> timeStrings(
        {
            "2018-02-03",
            "2018-02-07"
        }
    );

    std::vector<double> dataValues(
        {
            3.0,
            5.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::DAY);

    EXPECT_EQ(5, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-02-03"), resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(toTime_t("2018-02-04"), resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(toTime_t("2018-02-05"), resampler.resampledTimeSteps()[2]);
    EXPECT_EQ(toTime_t("2018-02-06"), resampler.resampledTimeSteps()[3]);
    EXPECT_EQ(toTime_t("2018-02-07"), resampler.resampledTimeSteps()[4]);

    EXPECT_NEAR(3.0, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(5.0, resampler.resampledValues()[1], 1e-12);
    EXPECT_NEAR(5.0, resampler.resampledValues()[2], 1e-12);
    EXPECT_NEAR(5.0, resampler.resampledValues()[3], 1e-12);
    EXPECT_NEAR(5.0, resampler.resampledValues()[4], 1e-12);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_WeightedMean_Decade)
{
    std::vector<QString> timeStrings(
        {
            "1999-02-03",
            "2005-06-06",
            "2012-02-07"
        }
    );

    std::vector<double> dataValues(
        {
            3.0,
            5.0,
            7.0
        }
    );

    time_t t0 = toTime_t("1999-02-03");
    time_t t1 = toTime_t("2005-06-06");
    time_t t2 = toTime_t("2012-02-07");
    time_t tp0 = toTime_t("2000-01-01");
    time_t tp1 = toTime_t("2010-01-01");
    time_t tp2 = toTime_t("2020-01-01");

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::DECADE);

    EXPECT_EQ(3, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(tp0, resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(tp1, resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(tp2, resampler.resampledTimeSteps()[2]);

    double value0 = 5.0 * (tp0 - t0) / (tp0 - toTime_t("1990-01-01"));
    double value1 = (5.0 * (t1 - tp0) + 7.0 * (tp1 - t1)) / (tp1 - tp0);
    double value2 = 7.0 * (t2 - tp1) / (tp2 - tp1);

    EXPECT_NEAR(value0, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value1, resampler.resampledValues()[1], 1e-12);
    EXPECT_NEAR(value2, resampler.resampledValues()[2], 1e-12);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_WeightedMean_SamplesStartBeforePeriod)
{
    std::vector<QString> timeStrings(
        {
            "2018-01-20",
            "2018-01-29",
            "2018-02-03",
            "2018-02-27",
            "2018-03-02"
        }
    );

    std::vector<double> dataValues(
        {
            3.0,
            5.0,
            7.0,
            11.0,
            13.0
        }
    );
    
    time_t tp0 = toTime_t("2018-02-01");
    time_t tp1 = toTime_t("2018-03-01");
    time_t tp2 = toTime_t("2018-04-01");

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::MONTH);

    EXPECT_EQ(3, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(tp0, resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(tp1, resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(tp2, resampler.resampledTimeSteps()[2]);

    double value0 =
        (5.0 * 9 +
         7.0 * 3) * SECS_PER_DAY / (tp0 - toTime_t("2018-01-01"));

    double value1 =
        (7.0 * 2 +
         11.0 * 24 +
         13.0 * 2) * SECS_PER_DAY / (tp1 - tp0);

    double value2 = 13.0 * 1 * SECS_PER_DAY / (tp2 - tp1);

    EXPECT_NEAR(value0, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value1, resampler.resampledValues()[1], 1e-12);
    EXPECT_NEAR(value2, resampler.resampledValues()[2], 1e-12);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_WeightedMean_SamplesStartBeforePeriod_TimeStepsMatchPeriod)
{
    std::vector<QString> timeStrings(
        {
            "2018-01-20",
            "2018-02-01",
            "2018-02-03",
            "2018-03-01",
            "2018-03-02"
        }
    );

    std::vector<double> dataValues(
        {
            3.0,
            5.0,
            7.0,
            11.0,
            13.0
        }
    );

    time_t tp0 = toTime_t("2018-02-01");
    time_t tp1 = toTime_t("2018-03-01");
    time_t tp2 = toTime_t("2018-04-01");

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::MONTH);

    EXPECT_EQ(3, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(tp0, resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(tp1, resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(tp2, resampler.resampledTimeSteps()[2]);

    double value0 =
        (5.0 * 12) * SECS_PER_DAY / (tp0 - toTime_t("2018-01-01"));

    double value1 =
        (7.0 * 2 +
         11.0 * 26) * SECS_PER_DAY / (tp1 - tp0);

    double value2 = 13.0 * 1 * SECS_PER_DAY / (tp2 - tp1);

    EXPECT_NEAR(value0, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value1, resampler.resampledValues()[1], 1e-12);
    EXPECT_NEAR(value2, resampler.resampledValues()[2], 1e-12);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_WeightedMean_SamplesStartAndEndMatchPeriod)
{
    std::vector<QString> timeStrings(
        {
            "2018-02-01",
            "2018-02-10",
            "2018-03-01"
        }
    );

    std::vector<double> dataValues(
        {
            3.0,
            5.0,
            7.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::MONTH);

    EXPECT_EQ(2, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-02-01"), resampler.resampledTimeSteps().front());
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps().back());

    time_t timePeriod = toTime_t("2018-03-01") - toTime_t("2018-02-01");

    double value1 = 3.0;
    double value2 =
        (5.0 * 9 +
         7.0 * 19) * SECS_PER_DAY / timePeriod;

    EXPECT_NEAR(value1, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value2, resampler.resampledValues()[1], 1e-12);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_WeightedMean_SamplesStartMatchPeriodStart)
{
    std::vector<QString> timeStrings(
        {
            "2018-02-01",
            "2018-02-10",
            "2018-03-01",
            "2018-03-02"
        }
    );

    std::vector<double> dataValues(
        {
            3.0,
            5.0,
            7.0,
            11.0
        }
    );

    time_t tp0 = toTime_t("2018-02-01");
    time_t tp1 = toTime_t("2018-03-01");
    time_t tp2 = toTime_t("2018-04-01");

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::MONTH);

    EXPECT_EQ(3, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(tp0, resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(tp1, resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(tp2, resampler.resampledTimeSteps()[2]);

    double value0 = 3.0;
    double value1 =
        (5.0 * 9 +
         7.0 * 19) * SECS_PER_DAY / (tp1 - tp0);
    double value2 = 11.0 * 1 * SECS_PER_DAY / (tp2 - tp1);

    EXPECT_NEAR(value0, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value1, resampler.resampledValues()[1], 1e-12);
    EXPECT_NEAR(value2, resampler.resampledValues()[2], 1e-12);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_WeightedMean_MultipleSamplesInLastPeriod)
{
    std::vector<QString> timeStrings(
        {
            "2018-02-10",
            "2018-03-02",
            "2018-03-05",
            "2018-03-15"
        }
    );

    std::vector<double> dataValues(
        {
            3.0,
            5.0,
            7.0,
            11.0
        }
    );

    time_t tp0 = toTime_t("2018-03-01");
    time_t tp1 = toTime_t("2018-04-01");

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::MONTH);

    EXPECT_EQ(2, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(tp0, resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(tp1, resampler.resampledTimeSteps()[1]);

    double value0 = 5.0 * 19 * SECS_PER_DAY / (tp0 - toTime_t("2018-02-01"));
    double value1 =
        (0.0 * 17 +
         11.0 * 10 +
         7.0 * 3 +
         5.0 * 1) * SECS_PER_DAY / (tp1 - tp0);

    EXPECT_NEAR(value0, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value1, resampler.resampledValues()[1], 1e-12);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_PeriodEndValues_SingleSample)
{
    std::vector<QString> timeStrings(
        {
            "2018-02-10"
        }
    );

    std::vector<double> dataValues(
        {
            3.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputePeriodEndValues(DateTimePeriod::MONTH);

    EXPECT_EQ(1, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps()[0]);

    EXPECT_NEAR(3.0, resampler.resampledValues()[0], 1e-12);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_PeriodEndValues_SamplesStartBeforePeriod)
{
    std::vector<QString> timeStrings(
        {
            "2018-01-30",
            "2018-02-10",
            "2018-03-05",
            "2018-03-02"
        }
    );

    std::vector<double> dataValues(
        {
            3.0,
            5.0,
            7.0,
            11.0
        }
    );

    time_t t0 = toTime_t("2018-01-30");
    time_t t1 = toTime_t("2018-02-10");
    time_t t2 = toTime_t("2018-03-05");
    time_t tp0 = toTime_t("2018-02-01");
    time_t tp1 = toTime_t("2018-03-01");
    time_t tp2 = toTime_t("2018-04-01");

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputePeriodEndValues(DateTimePeriod::MONTH);

    EXPECT_EQ(3, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(tp0, resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(tp1, resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(tp2, resampler.resampledTimeSteps()[2]);

    double value0 = (5.0 - 3.0) * (tp0 - t0) / (t1 - t0) + 3.0;
    double value1 = (7.0 - 5.0) * (tp1 - t1) / (t2 - t1) + 5.0;
    double value2 = 11.0;

    EXPECT_NEAR(value0, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value1, resampler.resampledValues()[1], 1e-12);
    EXPECT_NEAR(value2, resampler.resampledValues()[2], 1e-12);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_PeriodEndValues_SamplesStartMatchPeriod)
{
    std::vector<QString> timeStrings(
        {
            "2018-02-01",
            "2018-02-10",
            "2018-03-01",
            "2018-03-02"
        }
    );

    std::vector<double> dataValues(
        {
            3.0,
            5.0,
            7.0,
            11.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputePeriodEndValues(DateTimePeriod::MONTH);

    EXPECT_EQ(3, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-02-01"), resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps()[1]);
    EXPECT_EQ(toTime_t("2018-04-01"), resampler.resampledTimeSteps()[2]);

    EXPECT_NEAR(3.0, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(7.0, resampler.resampledValues()[1], 1e-12);
    EXPECT_NEAR(11.0, resampler.resampledValues()[2], 1e-12);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
TEST(RiaTimeHistoryCurveResampler, Test_PeriodEndValues_SamplesStartAndEndMatchPeriod)
{
    std::vector<QString> timeStrings(
        {
            "2018-02-01",
            "2018-02-10",
            "2018-03-01"
        }
    );

    std::vector<double> dataValues(
        {
            3.0,
            5.0,
            7.0
        }
    );

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputePeriodEndValues(DateTimePeriod::MONTH);

    EXPECT_EQ(2, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-02-01"), resampler.resampledTimeSteps().front());
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps().back());

    EXPECT_NEAR(3.0, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(7.0, resampler.resampledValues()[1], 1e-12);
}

