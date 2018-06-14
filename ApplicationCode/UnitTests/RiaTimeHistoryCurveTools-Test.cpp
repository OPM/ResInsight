#include "gtest/gtest.h"

#include "RiaTimeHistoryCurveResampler.h"
#include "RiaQDateTimeTools.h"


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

    EXPECT_EQ(0, (int)resampler.resampledTimeSteps().size());
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

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::DECADE);

    EXPECT_EQ(2, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2000-01-01"), resampler.resampledTimeSteps()[0]);
    EXPECT_EQ(toTime_t("2010-01-01"), resampler.resampledTimeSteps()[1]);

    time_t t1 = toTime_t("1999-02-03");
    time_t t2 = toTime_t("2005-06-06");
    time_t t3 = toTime_t("2012-02-07");
    time_t tp1 = toTime_t("2000-01-01");
    time_t tp2 = toTime_t("2010-01-01");

    double value1 = 5.0;
    double value2 = (5.0 * (t2 - tp1) + 7.0 * (tp2 - t2)) / (tp2 - tp1);

    EXPECT_NEAR(value1, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value2, resampler.resampledValues()[1], 1e-12);
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
    
    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::MONTH);

    EXPECT_EQ(2, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-02-01"), resampler.resampledTimeSteps().front());
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps().back());

    time_t timePeriod1 = toTime_t("2018-02-01") - toTime_t("2018-01-20");
    time_t timePeriod2 = toTime_t("2018-03-01") - toTime_t("2018-02-01");
    int timeDay = 60 * 60 * 24;
    
    double value1 =
        (5.0 * 9 +
         7.0 * 3) * timeDay / timePeriod1;

    double value2 =
        (7.0 * 2 +
         11.0 * 24 +
         13.0 * 2) * timeDay / timePeriod2;

    EXPECT_NEAR(value1, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value2, resampler.resampledValues()[1], 1e-12);
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

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::MONTH);

    EXPECT_EQ(2, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-02-01"), resampler.resampledTimeSteps().front());
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps().back());

    time_t timePeriod1 = toTime_t("2018-02-01") - toTime_t("2018-01-20");
    time_t timePeriod2 = toTime_t("2018-03-01") - toTime_t("2018-02-01");
    int timeDay = 60 * 60 * 24;

    double value1 =
        (5.0 * 12) * timeDay / timePeriod1;

    double value2 =
        (7.0 * 2 +
         11.0 * 26) * timeDay / timePeriod2;

    EXPECT_NEAR(value1, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value2, resampler.resampledValues()[1], 1e-12);
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
    int timeDay = 60 * 60 * 24;

    double value1 = 3.0;
    double value2 =
        (5.0 * 9 +
         7.0 * 19) * timeDay / timePeriod;

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

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputeWeightedMeanValues(DateTimePeriod::MONTH);

    EXPECT_EQ(2, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-02-01"), resampler.resampledTimeSteps().front());
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps().back());

    time_t timePeriod = toTime_t("2018-03-01") - toTime_t("2018-02-01");
    int timeDay = 60 * 60 * 24;

    double value1 = 3.0;
    double value2 =
        (5.0 * 9 +
         7.0 * 19) * timeDay / timePeriod;

    EXPECT_NEAR(value1, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value2, resampler.resampledValues()[1], 1e-12);
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

    RiaTimeHistoryCurveResampler resampler;
    resampler.setCurveData(dataValues, toTime_tVector(timeStrings));
    resampler.resampleAndComputePeriodEndValues(DateTimePeriod::MONTH);

    time_t t1 = toTime_t("2018-01-30");
    time_t t2 = toTime_t("2018-02-10");
    time_t t3 = toTime_t("2018-03-05");
    time_t t4 = toTime_t("2018-03-02");
    time_t tp1 = toTime_t("2018-02-01");
    time_t tp2 = toTime_t("2018-03-01");

    EXPECT_EQ(2, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-02-01"), resampler.resampledTimeSteps().front());
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps().back());

    double value1 = (5.0 - 3.0) * (tp1 - t1) / (t2 - t1) + 3.0;
    double value2 = (7.0 - 5.0) * (tp2 - t2) / (t3 - t2) + 5.0;

    EXPECT_NEAR(value1, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(value2, resampler.resampledValues()[1], 1e-12);
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

    EXPECT_EQ(2, (int)resampler.resampledTimeSteps().size());
    EXPECT_EQ(toTime_t("2018-02-01"), resampler.resampledTimeSteps().front());
    EXPECT_EQ(toTime_t("2018-03-01"), resampler.resampledTimeSteps().back());

    EXPECT_NEAR(3.0, resampler.resampledValues()[0], 1e-12);
    EXPECT_NEAR(7.0, resampler.resampledValues()[1], 1e-12);
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

