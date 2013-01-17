%% This is matlab file used to test/vizualize the various
%% distributions in trans_func.c

N = 100000;
x = random('normal',0 , 1 , N ,1);



y = trans_errf(x , 0.1 , 15 , -3 , 2.0);

disp(sprintf('<y> = %g' , mean(y)));
hist(y , sqrt(N))
