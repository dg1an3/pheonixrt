import pprint, math

def create_img(sz, fn):
    return [[float(fn(c,r)) for c in range(sz)] for r in range(sz)]

def flatten(img):
    return [px for row in img for px in row]

def calc_bins(img, width=.1):
    img = flatten(img)
    min, max = min(img), max(img)
    return min, max

def histogram(img, bins_at):
    img = flatten(img)
    print("histogram: img (flattened) = "); pprint.pprint(img)
    print("bins_at = "); pprint.pprint(bins_at)
    this_bin = 0.
    for (lower,upper) in zip(bins_at, bins_at[1:]):
        def frac(px):
            return (px-lower)/(upper-lower)
        high_fractions = list(map(frac, filter(lambda px:lower<=px and px<upper, img)))
        print(">>>> \thistogram: for {}: high_fractions = ".format((lower,upper))); pprint.pprint(high_fractions)
        this_bin += sum(map(lambda x:1.-x, high_fractions))
        yield ((lower, upper),this_bin)
        this_bin = sum(high_fractions)

def entropy(bins):
    eps = 1.e-6
    bin_sum = sum([count for _,count in bins])
    entropy = 0.0
    for ((lower,upper),count) in bins:
        p = max(count/bin_sum, eps)
        entropy += -p*math.log(p)
    return entropy   

if __name__ == '__main__':
    sz = 3
    sq_img = create_img(sz, lambda r,c:((r-sz//2)**2 + (c-sz//2)**2)/4.)
    print("sq_img ="); pprint.pprint(sq_img)

    #TODO: move this to function
    inf = 1.e+8
    bins_at = [-inf] + [(x/4.)-.125 for x in range(4)] + [inf]
    sq_img_bins = list(histogram(sq_img, bins_at))
    sq_img_entropy = entropy(sq_img_bins)
    print("sq_image histo ="); pprint.pprint(sq_img_bins)
    print("bin sum = {}".format(sum([count for _,count in sq_img_bins]))) 
    print("sq_image entropy = {}".format(sq_img_entropy))

    bright_img = create_img(sz, lambda r,c:sq_img[r][c]+0.1)
    print("bright_img ="); pprint.pprint(bright_img)
    bright_img_bins = list(histogram(bright_img, bins_at))
    bright_img_entropy = entropy(bright_img_bins)
    print("bright_img histo ="); pprint.pprint(bright_img_bins)
    print("bin sum = {}".format(sum([count for _,count in bright_img_bins]))) 
    print("bright_img entropy = {}".format(bright_img_entropy))

    #test that a uniform image has entropy >>> pixel count
    #test that a dirac image has entropy >>> ???
    #test that a gaussian image has entropy >>> ???
    #test that shifting pixel values for an image doesn't (significantly) change entropy, as bins get narrower
    #test d_Entropy / d_GaussWidth > 0.0

    #optimize entropy from dirac
    #optimize entropy from random
    #optimize entropy of an arbitrary image
