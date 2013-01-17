function y = trans_errf(x , min , max , skewness , width)

y = min + (max - min) * 0.5*(1 + erf((x + skewness)/(width * sqrt(2.0))));
