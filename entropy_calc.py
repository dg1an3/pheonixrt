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
    eps = 1e-3
    entropy = sum([-p*math.log(p+eps) for p in probs])
    return entropy

spike_image = np.zeros((3,3))
spike_image[1][1] = 1.0
print(spike_image)
print(entropy(mass_density(spike_image)))

flat_image = np.random.uniform(0.,1.,size=(3,3))
print(flat_image)
print(entropy(mass_density(flat_image)))

def num_grad(a, func, delta=1e-4):
    """
    evaluates the numeric gradient of a function
    """
    f_a = func(a)
    assert np.isscalar(f_a),  "func does not produce scalar"
    grad = np.zeros(a.shape)
    for r in range(a.shape[0]):
        for c in range(a.shape[1]):
            d_a = np.copy(a)
            d_a[r,c] = d_a[r,c] + delta
            grad[r,c] = (func(d_a) - f_a)/delta
    return grad

def entropy_grad(a):
    """
    """
    dEntropy_da = num_grad(a, lambda x:entropy(mass_density(x)), delta=1e-4)
    return dEntropy_da

def entropy_grad_norm(a):
    """
    """
    return np.linalg.norm(entropy_grad(a))

def maximize_entropy(a, lr=0.1, n=10):
    for _ in range(n):
        print('start a = ', a, entropy_grad(a))
        a = a + lr * entropy_grad(a)
        print('end a = ', a)
        a = a / sum(sum(a))
        print('normalized a = ', a)
        print('a entropy = ', entropy(mass_density(a)))