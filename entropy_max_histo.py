import matplotlib.pyplot as plt
import pprint, math, random, inspect

def create_img(sz, fn, dump=True):
    """
    creates a square image in the form of a sz*sz element list vector
    """
    img = [float(fn(c,r)) for c in range(sz) for r in range(sz)]
    if dump:
        print(inspect.getsource(fn))
        pprint.pprint(img)
    return img

def uniform_noise(delta=1e-3):
    return delta*(random.random()-0.5)

def calc_bins(img, width=.1):
    """
    """
    # min, max = math.min(img), math.max(img)
    inf = 1.e+2
    bins_at = [-inf] + [(x/16.)-.15 for x in range(24)] + [inf]
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

if __name__ == '__main__':

    sz = 29
    sq_img = create_img(sz, lambda r,c:uniform_noise()+((r-sz//2)**2 + (c-sz//2)**2)/1101., dump=False)

    plot_img = True
    if plot_img:
        plt.imshow([[sq_img[r*sz+c] for c in range(sz)] for r in range(sz)])
        plt.show()

    # create the bins to be reused
    bins_at = calc_bins(sq_img)
    sq_img_entropy = entropy_for_img(sq_img, bins_at, dump=False)
    print("original entropy = {}".format(sq_img_entropy))

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

    #optimize entropy from dirac
    #optimize entropy from random
    #optimize entropy of an arbitrary image
