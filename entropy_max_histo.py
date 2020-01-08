import matplotlib
import matplotlib.pyplot as plt
import pprint, math, random, inspect, numbers

def create_img(sz, fn, dump=True):
    """ creates a square image in the form of a sz*sz element list vector """
    img = [float(fn(c,r)) for c in range(sz) for r in range(sz)]
    if dump:
        print(inspect.getsource(fn))
        pprint.pprint(img)
    return img

def uniform_noise(delta=1e-3):
    return delta*(random.random()-0.5)

def calc_bins(img, width=.1):
    """ """
    min_px, max_px = min(img), max(img)
    min_bin = math.floor(min_px/width)*width
    bin_count = int((max_px - min_bin)/width)
    inf = 1.e+2
    bins_at = [-inf] + [x*width+min_bin 
                    for x in range(bin_count+2)] + [inf]
    return bins_at

def histogram(img, bins_at, dump=True):
    if dump:
        print("histogram: img (flattened) = "); pprint.pprint(img)
        print("bins_at = "); pprint.pprint(bins_at)
    this_bin = 0.
    for (lower,upper) in zip(bins_at, bins_at[1:]):
        def frac(px):
            return (px-lower)/(upper-lower)
        high_fractions = list(map(frac, filter(lambda px:lower<=px and px<upper, img)))
        if dump:
            print(">>>> \thistogram: for {}: high_fractions = ".format((lower,upper))); pprint.pprint(high_fractions)
        this_bin += sum(map(lambda x:1.-x, high_fractions))
        yield ((lower, upper),this_bin)
        this_bin = sum(high_fractions)

def entropy(bins):
    eps = 1e-10
    bin_sum = sum([count for _,count in bins])
    entropy = 0.0
    for ((lower,upper),count) in bins:
        p = max(count/bin_sum, eps)
        entropy += -p*math.log(p)
    return entropy

def entropy_for_img(img, bins_at, dump=True):
    img_bins = list(histogram(img, bins_at, dump))
    img_entropy = entropy(img_bins)
    if dump:
        print("image_histo ="); pprint.pprint(img_bins)
        print("bin sum = {}".format(sum([count for _,count in img_bins]))) 
        print("img_entropy = {}".format(img_entropy))
    return img_entropy

def mean_pixel_value_for_img(img):
    return sum(img)/float(len(img))

def num_grad(fn, v, delta=1.e-1):
    """
    """
    assert(len(v) > 0)
    for x in v: 
        assert isinstance(x, numbers.Number)
    fn_v = fn(v)
    def partial(n):
        delta_v = [x for x in v]
        delta_v[n] += delta   
        fn_delta_v = fn(delta_v)
        return (fn_delta_v - fn_v)/delta    
    jac = [partial(n) for n in range(len(v))]
    return jac

if __name__ == '__main__':

    sz = 5
    sq_img = create_img(sz, lambda r,c:uniform_noise()+((r-sz//2)**2 + (c-sz//2)**2)/1101., dump=False)

    plt.ion()
    plot_img = True
    if plot_img:
        plt.imshow([[sq_img[r*sz+c] for c in range(sz)] for r in range(sz)])
        plt.draw()
        input()
    plt.close()

    # create the bins to be reused
    bins_at = calc_bins(sq_img)
    sq_img_entropy = entropy_for_img(sq_img, bins_at, dump=False)
    print("original entropy = {}".format(sq_img_entropy))

    test_step = False
    if test_step:
        step = 1e-10
        for brighten in range(1,100):
            bright_img = create_img(sz, lambda r,c:sq_img[r*sz+c]+step*brighten, dump=False)
            bright_img_entropy = entropy_for_img(bright_img, bins_at, dump=False)
            dentropy = (bright_img_entropy - sq_img_entropy)/(step*brighten)
            print("dentropy = {}\tupdated entropy = {}".format(dentropy, bright_img_entropy))

    #test that a uniform image has entropy >>> pixel count
    #test that a dirac image has entropy >>> ???
    #test that a gaussian image has entropy >>> ???
    #test that shifting pixel values for an image doesn't (significantly) change entropy, as bins get narrower
    #test d_Entropy / d_GaussWidth > 0.0

    print("initial img = "); pprint.pprint(sq_img)
    for iter in range(100):
        dS_dPixels = num_grad(lambda v:(mean_pixel_value_for_img(v)-0.5)**2-entropy_for_img(v, bins_at, dump=False), sq_img)
        print("dS_dPixels = "); pprint.pprint(dS_dPixels)
        sq_img = list(map(lambda px_dS:px_dS[0]-((1e-1)*px_dS[1]),
                            zip(sq_img, dS_dPixels)))
        print("next img = "); pprint.pprint(sq_img)

    if plot_img:
        plt.imshow([[sq_img[r*sz+c] for c in range(sz)] for r in range(sz)])
        plt.draw()
        input()

    #optimize entropy from dirac
    #optimize entropy from random
    #optimize entropy of an arbitrary image

