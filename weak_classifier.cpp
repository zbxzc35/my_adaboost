#include "weak_classifier.h"
#include "tool.h"

typedef struct {
    float value;
    int label;
    float weight;
} Scores;


#define LT(a, b) ((a).value < (b).value)

static IMPLEMENT_QSORT(sort_arr_scores, Scores, LT);

void init_weak_classifier(WeakClassifier *weak, float thresh, int sign, Feature *feat)
{
    weak->thresh = thresh;
    weak->sign = sign;
    weak->feat = feat;
}


float train(WeakClassifier *weak, float *value, int posSize, int negSize, float *weights)
{
    int sampleSize = posSize + negSize;
    Scores scores[MAX_SAMPLE_SIZE];
    float wp[MAX_SAMPLE_SIZE], wn[MAX_SAMPLE_SIZE];

    float tp, tn;

    float minError = 1;
    float errorPos = 0;
    float errorNeg = 0;

    int i;

    for(i = 0; i < posSize; i++)
    {
        scores[i].value = value[i];
        scores[i].label = 1;
        scores[i].weight = weights[i];
    }

    for(; i < sampleSize; i++)
    {
        scores[i].value = value[i];
        scores[i].label = 0;
        scores[i].weight = weights[i];
    }

    sort_arr_scores(scores, sampleSize);

    if(scores[0].label == 1)
    {
        wp[0] = 0;
        wn[0] = scores[0].weight;
    }
    else
    {
        wn[0] = scores[0].weight;
        wp[0] = 0;
    }

    for(i = 1; i < sampleSize; i++)
    {
        if(scores[i].label == 1)
        {
            wp[i] = wp[i - 1] + scores[i].weight;
            wn[i] = wn[i - 1];
        }
        else
        {
            wp[i] = wp[i - 1];
            wn[i] = wn[i - 1] + scores[i].weight;
        }
    }

    tp = wp[sampleSize - 1];
    tn = wn[sampleSize - 1];


    for(i = 0; i < sampleSize; i++)
    {
        errorPos = wp[i] + tn - wn[i];
        errorNeg = wn[i] + tp - wp[i];

        if(errorPos < errorNeg && errorPos < minError)
        {
            minError = errorPos;
            weak->sign = 1;
            weak->thresh = scores[i].value;
        }
        else if(errorNeg < errorPos && errorNeg < minError)
        {
            minError = errorNeg;
            weak->sign = 0;
            weak->thresh = scores[i].value;
        }
    }

    return minError;
}


int classify(WeakClassifier *weak, float *img, int stride, int x, int y)
{
    float value = get_value(weak->feat, img, stride, x, y);
    float thresh = weak->thresh;
    float sign = weak->sign;

    if( (value > thresh && sign == 1) || (value <= thresh && sign == 0) )
        return 1;

    else
        return -1;
}


static int classify(WeakClassifier *weak, float *values, int size)
{
    int res = 0;
    float thresh = weak->thresh;
    float sign = weak->sign;

    for(int i = 0; i < size; i++)
        res += ( (values[i] > thresh  && sign == 1) || (values[i] <= thresh && sign == 0) );

    return res;
}


float test_weak_classifier(float *posSample, int numPos, float *negSample, int numNeg)
{
    WeakClassifier wc;

    int sampleSize = numPos + numNeg;

    float *values = new float[sampleSize];
    float *weights = new float[sampleSize];

    float factor = 0.5 / numPos;

    for(int i = 0; i < numPos; i++)
    {
        values[i] = posSample[i];
        weights[i] = factor;
    }

    factor = 0.5 / numNeg;

    for(int i = numPos; i < sampleSize; i++)
    {
        values[i] = negSample[i - numPos];
        weights[i] = factor;
    }

    train(&wc, values, numPos, numNeg, weights);

    int cpos = classify(&wc, posSample, numPos) ;
    int cneg = numNeg - classify(&wc, negSample, numNeg);


    return 100.0 * (cpos + cneg) / (numPos + numNeg);
}
