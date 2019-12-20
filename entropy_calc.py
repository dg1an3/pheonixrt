import numpy as np

def pairwise(iterable):
    from itertools import tee
    "s -> (s0,s1), (s1,s2), (s2, s3), ..."
    a, b = tee(iterable)
    next(b, None)
    return zip(a, b)

def mass_density(a):
    bins, edges = np.histogram(a, density=True)
    probs = [bin*(max-min) for bin, (min,max) 
                in zip(bins, pairwise(edges))]
    return probs

def entropy(probs):
    import math
    eps = 1e-6
    entropy = sum([-p*math.log(p+eps) for p in probs])
    return entropy

spike_image = np.zeros((3,3))
spike_image[1][1] = 1.0
print(spike_image)
print(entropy(mass_density(spike_image)))

flat_image = np.random.uniform(0.,1.,size=(3,3))
print(flat_image)
print(entropy(mass_density(flat_image)))