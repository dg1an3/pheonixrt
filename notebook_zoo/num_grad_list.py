import numbers

def num_grad(fn, vec, delta=1.e-4):
    """
    """
    assert(len(vec) > 0)
    for x in vec: 
        assert isinstance(x, numbers.Number)
    fn_vec = fn(vec)
    def partial(n):
        delta_vec = vec.copy()
        delta_vec[n] += delta        
        fn_delta_vec = fn(delta_vec)
        return (fn_delta_vec - fn_vec)/delta    
    jac = [partial(n) for n in range(len(vec))]
    return jac

if __name__ == '__main__':
    fn = lambda v:[v*v]
    print(num_grad(fn,[0.,0.,0.]))
