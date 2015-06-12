#pragma once


namespace caf
{
template< typename S>
class Tensor3D
{
    S v[6];
public:
    void setFromAbaqusLayout(S* tensorData) 
    { 
        v[0] = tensorData[0]; 
        v[1] = tensorData[1]; 
        v[2] = tensorData[2]; 
        v[3] = tensorData[3]; 
        v[4] = tensorData[4]; 
        v[5] = tensorData[5]; 
    }
};

typedef Tensor3D<float> Ten3Df;

}


