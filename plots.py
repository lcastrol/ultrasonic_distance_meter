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

def plot():

    #Set the style
    matplotlib.style.use('ggplot')

    args = parse_arguments()
    destiny = args.destiny

    # Create folder if it doesn't exits
    if not os.path.isdir(destiny):
        os.makedirs(destiny)

    if args.plot_histogram == True:

        data,total_samples = get_sample(args.data_file)

        game_max_scores = []

        for element in range(len(data)):
            for data_point in range(len(data[element])): 
                game_max_scores.append(data[element][data_point]) 

        print total_samples 
        plot_percentages(game_max_scores, destiny);
        plot_histogram(game_max_scores, destiny)

    
    if args.plot_pdf == True:

        # Open data file
        iFile = open(args.data_file, 'rb')
        #reader = csv.reader(filter(lambda row: row[0]!='#', iFile))
        reader = csv.reader(iFile)

        game_score_list = []
        episode_number_list = []

        score_column = 5
        episode_column = 0

        # Read values from csv file
        prev_episode = '0'
        prev_row = []
        for row in reader:
            if prev_episode != row[0]:
                game_score_list.append(int(prev_row[score_column]))
                episode_number_list.append(int(prev_row[episode_column]))
            prev_row = row
            prev_episode = row[0]

        episode_number = np.array(episode_number_list)
        game_score = np.array(game_score_list)

        plot_pdf(game_score, destiny)

        iFile.close()

    if args.plot_score_evo == True:

        # Open data file
        iFile = open(args.data_file, 'rb')
        #reader = csv.reader(filter(lambda row: row[0]!='#', iFile))
        reader = csv.reader(iFile)

        game_score_list = []
        episode_number_list = []

        score_column = 5
        episode_column = 0

        # Read values from csv file
        prev_episode = '0'
        prev_row = []
        for row in reader:
            # Search for the last score in the episode
            if prev_episode != row[0]:
                game_score_list.append(int(prev_row[score_column]))
                episode_number_list.append(int(prev_row[episode_column]))
            prev_row = row
            prev_episode = row[0]

        episode_number = np.array(episode_number_list)
        game_score = np.array(game_score_list)

        plot_frame_skip(episode_number, game_score, destiny)

        iFile.close()

    if args.analyze_sample == True:

        data, _ = get_sample(args.data_file)
        plot_box(data,destiny)


def get_sample(files_list):

    # Open file with a list of the results file names
    files_names = open(files_list, 'rb')

    game_max_scores = []

    for file_name in files_names:

        # Open data file
        iFile = open(file_name.rstrip(), 'rb')
        #reader = csv.reader(filter(lambda row: row[0]!='#', iFile))
        file_handler = csv.reader(iFile)

        game_score_list = []
        episode_number_list = []

        score_column = 5
        episode_column = 0

        # Read values from csv file
        prev_episode = '0'
        prev_row = []
        for row in file_handler:
            if ((len(row) != 9) or (len([x for x in row if ((x == '') or (x == '-')) ]) != 0)): #TODO remove this, log fixed
                continue 
            # Search for the last score in the episode
            if prev_episode != row[0]:
                game_score_list.append(int(prev_row[score_column]))
                episode_number_list.append(int(prev_row[episode_column]))
            prev_row = row
            prev_episode = row[0]

        episode_number = np.array(episode_number_list)
        game_max_scores.append(np.array(game_score_list))

        #for row in file_handler:
        #    all_data = [np.random.normal(0, std, 100) for std in range(6, 10)]

        #Close the file

        iFile.close()

    return game_max_scores , len(game_max_scores) 

def plot_box(all_data,destiny):

    fig, ax = plt.subplots(nrows=1, ncols=1, figsize=(6, 5))

    # plot box plot
    ax.boxplot(all_data)
    ax.set_title('box plot')

    # adding horizontal grid lines
    ax.yaxis.grid(True)
    ax.set_xticks([y+1 for y in range(len(all_data))])
    ax.set_xlabel('Sample set')
    ax.set_ylabel('Differential score')

    # add x-tick labels
    plt.setp(ax, xticks=[y+1 for y in range(len(all_data))],
             xticklabels=['set 1', 'set 2', 'set 3', 'set 4'])

    plt.grid(True)
    plt.title('Game scores distributions', y=1.08)
    plt.subplots_adjust(top=0.82)

    plt.savefig("%s/box_plot.png" % destiny)
    print "Box plot printed \n"
    plt.close()

def plot_percentages(data, destiny):

    win =0
    lose = 0
    tie = 0

    for result in range(len(data)): 
        if data[result] > 0:
            lose += 1
        if data[result] == 0:
            tie += 1
        if data[result] < 0:
            win += 1
    
    win = 100*win/len(data)
    lose = 100*lose/len(data)
    tie = 100*tie/len(data)

    # Plot histogram
    fig, ax = plt.subplots()

    ind = np.arange(1, 4)
    ba1, ba2, ba3 = plt.bar(ind,(tie,win,lose))

    centers = ind + 0.5*ba1.get_width()

    ax.set_xlim([0.5, 4])
    ax.set_xticks(centers)
    ax.set_ylim([0, 100])
    ax.set_xticklabels(['Draw', 'SARSA wins', 'SARSA lose'])

    plt.xlabel('Results')
    plt.title('Final game result percentually')
    plt.savefig("%s/game_score_percentages.png" % destiny)
    plt.close()

def plot_histogram(data, destiny):

    # Plot histogram
    plt.hist(data, bins=range(min(data)-1,max(data)+1), color='g', normed=True)

    plt.xlim(min(data)-1,max(data)+1)

    plt.xlabel('Final score difference (Player A - Player B)')
    plt.title('Game score histogram')
    plt.savefig("%s/game_score_histogram.png" % destiny)
    plt.close()

def plot_pdf(data, destiny):


    # Plot histogram
    plt.hist(data, bins=range(min(data),max(data)), color='g', normed=True)

    ##Fit a normal distribution
    best_fit_dist_name, best_fit_params = best_fit_distribution(data, bins=range(min(data),max(data)))
    #dist = getattr(st, 'norm')
    dist = getattr(st, best_fit_dist_name)
    #param = dist.fit(data)
    #param = best_fit_params

    # Make PDF
    pdf = make_pdf(dist, best_fit_params)
    pdf.plot(lw=2, label='PDF', legend=True)

    #For a normal distribution the keyword parameter loc defines the mean and the keyword parameter scale defines the standard deviation.
    #pdf_fitted = dist.pdf(scipy.arange(-len(data)/2,len(data)/2), *param[:-2], loc=param[-2], scale=param[-1]) * len(data)
    plt.xlim(min(data)-1,max(data)+1)
    #plt.plot(scipy.arange(-len(data)/2,len(data)/2), pdf_fitted, 'r-', label='normal dist')

    param_names = (dist.shapes + ', loc, scale').split(', ') if dist.shapes else ['loc', 'scale']
    param_str = ', '.join(['{}={:0.2f}'.format(k,v) for k,v in zip(param_names, best_fit_params)])
    dist_str = '{}'.format(param_str)

    mean, variance = dist.stats(*best_fit_params[:-2],moments='mv')
    median = dist.median(*best_fit_params[:-2],loc=best_fit_params[-2],scale=best_fit_params[-1])

    plt.xlabel('Final score difference (Player A - Player B)')
    plt.title(('Game score histogram \n Best fitting distribution: %s \n Mean = %.2f, Variance = %.2f, Median = %.2f \n' % (best_fit_dist_name, mean,variance,median)) + dist_str)
    plt.subplots_adjust(top=0.82)
    plt.grid(True)
    plt.savefig("%s/game_score_histogram.png" % destiny)
    plt.close()

# Create models from data
def best_fit_distribution(data, bins=200, ax=None):
    """Model data by finding best fit distribution to data"""
    # Get histogram of original data
    y, x = np.histogram(data, bins=bins, normed=True)
    x = (x + np.roll(x, -1))[:-1] / 2.0

    # Distributions to check
    DISTRIBUTIONS = [
        st.alpha,st.anglit,st.arcsine,st.beta,st.betaprime,st.bradford,st.burr,st.cauchy,st.chi,st.chi2,st.cosine,
        st.dgamma,st.dweibull,st.erlang,st.expon,st.exponnorm,st.exponweib,st.exponpow,st.f,st.fatiguelife,st.fisk,
        st.foldcauchy,st.foldnorm,st.frechet_r,st.frechet_l,st.genlogistic,st.genpareto,st.gennorm,st.genexpon,
        st.genextreme,st.gausshyper,st.gamma,st.gengamma,st.genhalflogistic,st.gilbrat,st.gompertz,st.gumbel_r,
        st.gumbel_l,st.halfcauchy,st.halflogistic,st.halfnorm,st.halfgennorm,st.hypsecant,st.invgamma,st.invgauss,
        st.invweibull,st.johnsonsb,st.johnsonsu,st.ksone,st.kstwobign,st.laplace,st.levy,st.levy_l,st.levy_stable,
        st.logistic,st.loggamma,st.loglaplace,st.lognorm,st.lomax,st.maxwell,st.mielke,st.nakagami,st.ncx2,st.ncf,
        st.nct,st.norm,st.pareto,st.pearson3,st.powerlaw,st.powerlognorm,st.powernorm,st.rdist,st.reciprocal,
        st.rayleigh,st.rice,st.recipinvgauss,st.semicircular,st.t,st.triang,st.truncexpon,st.truncnorm,st.tukeylambda,
        st.uniform,st.vonmises,st.vonmises_line,st.wald,st.weibull_min,st.weibull_max,st.wrapcauchy
    ]

    # Best holders
    best_distribution = st.norm
    best_params = (0.0, 1.0)
    best_sse = np.inf

    # Estimate distribution parameters from data
    for distribution in DISTRIBUTIONS:

        # Try to fit the distribution
        try:
            # Ignore warnings from data that can't be fit with warnings.catch_warnings():
                #warnings.filterwarnings('ignore')

                # fit dist to data
                params = distribution.fit(data)

                # Separate parts of parameters
                arg = params[:-2]
                loc = params[-2]
                scale = params[-1]

                # Calculate fitted PDF and error with fit in distribution
                pdf = distribution.pdf(x, loc=loc, scale=scale, *arg)
                sse = np.sum(np.power(y - pdf, 2.0))

                # if axis pass in add to plot
                try:
                    if ax:
                        pd.Series(pdf, x).plot(ax=ax)
                    end
                except Exception:
                    pass

                # identify if this distribution is better
                if best_sse > sse > 0:
                    best_distribution = distribution
                    best_params = params
                    best_sse = sse

        except Exception:
            print 'error out'
            pass

    return (best_distribution.name, best_params)

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

# plot average difference / frame skip opponent graphs with fixed frame skip of frame_skip
def plot_frame_skip(x_list, y_list, destiny):

    plt.plot(x_list, y_list, 'ro', label='Game Score')

    legend = plt.legend(loc='lower left', shadow=True)
    plt.xlabel('frame skip opponent (frames)')
    plt.ylabel('average result difference (points)')
    plt.title('Evolution of the game score')
    plt.grid(True)

    plt.savefig("%s/game_score_evolution.png" % destiny)
    plt.close()

def parse_arguments():

    parse = argparse.ArgumentParser()

    parse.add_argument("data_file", type=str, help="data file in csv format, or a file with the list of files to be read by the box graph")
    parse.add_argument("--destiny", type=str, default="./plot", help="real path to the destiny folder to save the plots, if it doesn't exists it will be created")

    parse.add_argument("--score-evolution", dest="plot_score_evo", action='store_true', default=False, help="Generate graphs of the score evolution ")
    parse.add_argument("--histogram", dest="plot_histogram", action='store_true', default=False, help="Generate a histogram")
    parse.add_argument("--mean", dest="analyze_sample", action='store_true', default=False, help="Generate a box graph")
    parse.add_argument("--pdf", dest="plot_pdf", action='store_true', default=False, help="Plot best fit")

    #Parse the arguments
    args = parse.parse_args()
    return args

# Execute the program
if __name__ == "__main__":
    plot()
