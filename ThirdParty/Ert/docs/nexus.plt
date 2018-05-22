

bbbbPLOT__BIN_           : Header - verify file type
562*b                    : Header - skipped
264*b                    : Header - skipped
i                        : num_classes
iii                      : Day Month Year
i                        : nx
i                        : ny
i                        : nz
i                        : ncomp (number of phases???)
8b                       : ???
num_classes *8b          : The class names
8*b                      : Skipped
num_classes *i           : The number of variables in each class (var_in_class[])
8*b                      : Skipped

for c in classes:
    4*b                  : TIME var name
    for v in vars[c]:
         4*b             : The variable names
    8*b                  : Skipped

---->                    : This is the position of first timestep

while classname != STOP:
    8*b                      : classname
    8*b                      : skipped
    f                        : timestep  (cast to int)
    f                        : time
    f                        : num_items (cast to int)
    f                        : max_items (cast to int)
    f                        : max_perfs (cast to int)

    if classame in ["WELLBORE","WELLYR"]:
       8*b                   : skipped
       for item in num_items:
           8*b               : skipped
           8*b               : skipped
           8*b               : ncon = atoi( )
          48*b               : skipped
          for con in ncon:
              8*b            : con_num (ignored)
              8*b            : layer_name (ignored)
              var_in_class*f : The real data
              16*b           : skipped
    else:
        8*b                  : skipped
       72*b                  : skipped
       var_in_class*f        : The real data
    8*b                      : skipped
