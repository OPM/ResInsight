
uniform int u_clipPlaneCount;

varying float v_clipDist[6];

#define CVF_CHECK_DISCARD_FRAGMENT_IMPL

//--------------------------------------------------------------------------------------------------
/// Check if fragment should be discarded based on clip distances and discard if needed
//--------------------------------------------------------------------------------------------------
void checkDiscardFragment()
{
	int i;
	for (i = 0; i < u_clipPlaneCount; i++)
	{
		if (v_clipDist[i] < 0.0) discard;
	}
}
