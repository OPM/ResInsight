import rips

resInsight  = rips.Instance.find()

if resInsight is None:
    print('ERROR: could not find ResInsight')
else:
	print('Successfully connected to ResInsight')