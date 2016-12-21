#!/usr/bin/python2

import os
import csv
import numpy as np
import scipy
import scipy.stats as st
import pandas as pd
import matplotlib
import matplotlib.pyplot as plt
import statsmodels as sm
import matplotlib.patches as patches
import matplotlib.path as path
import argparse
import math

def plot():
    #Set the style
    matplotlib.style.use('ggplot')

    args = parse_arguments()
    # Create folder if it doesn't exits

    destiny = "./plot"
    if not os.path.isdir(destiny):
        os.makedirs(destiny)
    # Open data file
    file_name = args.data_file
    iFile = open(file_name, 'rb')

    data = []

    for row in iFile:
        data.append(int(row.rstrip()))

    # Plot histogram
    plt.hist(data, bins=range(min(data)-1,max(data)+1), color='g', normed=True)

    #distribution = st.norm
    #params = distribution.fit(data)
    #dist = getattr(st, 'norm')

    #mean, variance = dist.stats(*params[:-2],moments='mv')
    #median = dist.median(*params[:-2])

    #plt.xlabel('Final score difference (Player A - Player B)')
    #pdf = make_pdf(dist, params)
    #pdf.plot(lw=2, label='PDF', legend=True)
    plt.xlim(min(data)-2,max(data)+2)
    #plt.xticks(np.arange(min(data),max(data)))

    #plt.title('Data back from the sensor\n Mean = %.2f, Variance = %.2f, Median = %.2f \n' % (mean,variance,median))
    #plt.xlabel('readings')
    plt.title('')
    plt.savefig("%s/plot.png" % destiny)
    plt.close()

def make_pdf(dist, params, size=10000):
    """Generate distributions's Propbability Distribution Function """

    # Separate parts of parameters
    arg = params[:-2]
    loc = params[-2]
    scale = params[-1]

    # Get sane start and end points of distribution
    start = dist.ppf(0.01, *arg, loc=loc, scale=scale) if arg else dist.ppf(0.01, loc=loc, scale=scale)
    end = dist.ppf(0.99, *arg, loc=loc, scale=scale) if arg else dist.ppf(0.99, loc=loc, scale=scale)

    # Build PDF and turn into pandas Series
    x = np.linspace(start, end, size)
    y = dist.pdf(x, loc=loc, scale=scale, *arg)
    pdf = pd.Series(y, x)

    return pdf

def parse_arguments():

    parse = argparse.ArgumentParser()

    parse.add_argument("data_file", type=str, help="input file")

    #Parse the arguments
    args = parse.parse_args()
    return args

#Execute the program
if __name__ == "__main__":
    plot()
