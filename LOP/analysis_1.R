# Expected CSV format:
# start, neighbourhood, pivot, cost, time_in_seconds, deviation_percentage

directory <- "C:\\Users\\benja\\OneDrive\\Bureaublad\\vub\\master_ai\\semester_2\\heuristic\\task\\LOP\\x64\\Debug"

# Usage in R:
# echo = FALSE -> R will not print the commands (red text).
# source("C:\\Users\\benja\\OneDrive\\Bureaublad\\vub\\master_ai\\semester_2\\heuristic\\task\\LOP\\LOP\\analysis_1.R", echo = FALSE)

# --- 0. Get all CSV files in the folder -------------------------------------
# full.names = TRUE -> returns full paths instead of just filenames.
# files is a vector of file paths.
files <- list.files(paste0(directory, "\\implementation_1_results"), pattern = "\\.csv$", full.names = TRUE)
# rbind = row bind.
# It stacks data frames on top of each other.
# rbind normally expects arguments like: rbind(df1, df2, df3).
# Currently: list(df1, df2, df3).
# do.call allows us to call rbind on a list of data frames, effectively doing rbind(df1, df2, df3) without having to name them individually.
# lapply = loop over a list/vector and apply a function to each element.
data <- do.call(rbind, lapply(files, function(f) {
    # In R, stringsAsFactors=FALSE tells read.csv() to keep text columns as plain character strings instead of converting them to factors.
    # A factor is R's way of representing categorical data.
    # It stores values as integer codes with labels. For example, "male"/"female" stored as 1/2 internally.
    df <- read.csv(f, stringsAsFactors = FALSE)
    # Extract filename without path and extension
    instance_name <- tools::file_path_sans_ext(basename(f))
    # Add instance column
    df$instance <- instance_name
    return(df)
}))

# $ is used to access a column of a data frame by name.
# In this case, data$start returns a vector of all the values in the start column.
# You can both add or change a column using this syntax.
# Create a new column called algorithm by combining start, neighbourhood, and pivot.

# paste() joins multiple strings together.
# It takes the values from three columns row by row and joins them with "_" as a separator.
# So each row gets a unique label that combines all three column values into one identifier string.

# trimws removes any leading and trailing whitespace from strings.
# This is a safety step in case any of the source columns ("start", "neighbourhood", "pivot") had accidental spaces.
data$algorithm <- paste(trimws(data$start), trimws(data$neighbourhood), trimws(data$pivot), sep = "_")

# nrow(data) counts the total number of rows in the dataframe.
# length(unique(data$algorithm)) counts distinct algorithms.
cat("Loaded", nrow(data), "rows covering",
    length(unique(data$instance)), "instances and",
    length(unique(data$algorithm)), "algorithms.\n\n")

# --- 1. Summary statistics per algorithm -------------------------------------
cat("=== Average percentage deviation from best known (%) ===\n")
# The ~ in R is called a formula operator, and it’s used to describe a relationship between variables.
# In this case, deviation_percentage ~ algorithm means group deviation_percentage by algorithm.

# Groups the dataset by algorithm.
# Computes the mean of deviation_percentage for each algorithm.
avg_dev <- aggregate(deviation_percentage ~ algorithm, data = data, FUN = mean)
# Sorts the results by deviation_percentage in ascending order.
avg_dev <- avg_dev[order(avg_dev$deviation_percentage), ]
# Prints the table without row numbers.
print(avg_dev, row.names = FALSE)

cat("\n=== Standard deviation of percentage deviation per algorithm (%) ===\n")
sd_dev <- aggregate(deviation_percentage ~ algorithm, data = data, FUN = sd)
sd_dev <- sd_dev[order(sd_dev$deviation_percentage), ]
print(sd_dev, row.names = FALSE)

cat("\n=== Standard deviation of computation time per algorithm (seconds) ===\n")
sd_time <- aggregate(time_in_seconds ~ algorithm, data = data, FUN = sd)
sd_time <- sd_time[order(sd_time$time_in_seconds), ]
print(sd_time, row.names = FALSE)

cat("\n=== Total computation time per algorithm (seconds) ===\n")
total_time <- aggregate(time_in_seconds ~ algorithm, data = data, FUN = sum)
total_time <- total_time[order(total_time$time_in_seconds), ]
print(total_time, row.names = FALSE)

# --- 2. Pairwise statistical tests -------------------------------------------
# Compare every pair of algorithms using:
# - Student t-test
# - Wilcoxon signed-rank test

algorithms <- sort(unique(data$algorithm))
n_alg <- length(algorithms)
instances <- sort(unique(data$instance))

# Creates a matrix filled with NA (missing values).
# dimnames = list(instances, algorithms) assigns row names and column names to the matrix.
# So the matrix looks conceptually like:
#           AlgA   AlgB   AlgC
# inst1      NA     NA     NA
# inst2      NA     NA     NA
# inst3      NA     NA     NA
dev_matrix <- matrix(NA, nrow = length(instances), ncol = n_alg,
                     dimnames = list(instances, algorithms))

for (inst in instances) {
    for (alg in algorithms) {
        # For each instance and algorithm, look up the corresponding deviation_percentage value in the original data.
        val <- data$deviation_percentage[data$instance == inst & data$algorithm == alg]
        if (length(val) == 1)
            dev_matrix[inst, alg] <- val
    }
}

# Pairwise t-test p-value matrix

# the paired t-test is used to compare two algorithms on the same problem instances:
# Null hypothesis (H0): The mean difference in deviation_percentage between the two algorithms is 0 (-> they perform the same on average)
# Alternative hypothesis (H₁): The mean difference is not 0 (-> they perform differently)
# Because paired = TRUE, it's doing this instance-by-instance comparison, not treating them as independent samples.
# p-value < 0.05 -> Reject H0 -> Algorithms are significantly different
# p-value => 0.05 -> Fail to reject H0 -> No evidence of a significant difference (they might be similar)

# This matrix is symmetric!
cat("\n=== Pairwise Student t-test p-values (paired, two-sided) ===\n")
ttest_p <- matrix(NA, n_alg, n_alg, dimnames = list(algorithms, algorithms))
for (a1 in algorithms) {
    for (a2 in algorithms) {
        # When comparing an algorithm with itself, the p-value is set to 1 (perfectly “no difference”).
        # next skips to the next iteration.
        if (a1 == a2) { ttest_p[a1, a2] <- 1; next }
        # x -> deviations for algorithm a1 across all instances.
        # y -> deviations for algorithm a2 across all instances.
        x <- dev_matrix[, a1]
        y <- dev_matrix[, a2]
        # Logical vector TRUE for rows where both x and y are not NA.
        # Ensures only instances where both algorithms have valid data are compared.
        # So the ok looks conceptually like: [TRUE, FALSE, TRUE].
        ok <- complete.cases(x, y)
        if (sum(ok) >= 2) {
            # x[ok] = subset of x where ok = TRUE.
            # y[ok] = subset of y where ok = TRUE.

            # Paired t-test -> compares the same instances between two algorithms.
            # paired = TRUE -> accounts for correlation because the same instances are used.
            # Default is two-sided, checking if means are different in either direction.
            res <- t.test(x[ok], y[ok], paired = TRUE)
            # res$p.value -> extracts the p-value of the t-test.
            # Store it in the ttest_p matrix at [a1, a2].
            ttest_p[a1, a2] <- res$p.value
        }
    }
}
print(ttest_p)

# Pairwise Wilcoxon signed-rank test p-value matrix
# This matrix is symmetric!
cat("\n=== Pairwise Wilcoxon signed-rank test p-values (paired, two-sided) ===\n")
wilcox_p <- matrix(NA, n_alg, n_alg, dimnames = list(algorithms, algorithms))
for (a1 in algorithms) {
    for (a2 in algorithms) {
        if (a1 == a2) { wilcox_p[a1, a2] <- 1; next }
        x <- dev_matrix[, a1]
        y <- dev_matrix[, a2]
        ok <- complete.cases(x, y)
        if (sum(ok) >= 2) {
            # This is a built-in R function used to perform the Wilcoxon signed-rank test (or the Wilcoxon rank-sum test if unpaired).
            # The Wilcoxon test can calculate an exact p-value for small sample sizes.
            # Setting exact = FALSE forces R to approximate the p-value using a normal distribution (z-approximation).
            # This is often done for larger datasets where computing the exact p-value is too slow.
            res <- wilcox.test(x[ok], y[ok], paired = TRUE, exact = FALSE)
            wilcox_p[a1, a2] <- res$p.value
        }
    }
}
print(wilcox_p)

cat("\n(p < 0.05 indicates a statistically significant difference at the 5% level)\n")

# --- 3. Significance summary -------------------------------------------------
alpha <- 0.05
cat("\n=== Pairs with significant difference (Wilcoxon, p <", alpha, ") ===\n")
found <- FALSE
for (a1 in algorithms) {
    for (a2 in algorithms) {
        # Avoid repeating the same pair twice.
        # Compare every algorithm against every other algorithm exactly once.
        if (a1 >= a2) next
        p <- wilcox_p[a1, a2]
        # If the p-value is not NA and less than the significance level alpha, 
        # Consider this pair to have a statistically significant difference in performance.
        if (!is.na(p) && p < alpha) {
            # na.rm = TRUE -> computes the average deviation, ignoring missing values (NA).
            # Compute the average deviation for a1 and a2.
            # If a1’s average deviation is smaller than a2’s, then better = a1, else better = a2.
            better <- ifelse(mean(dev_matrix[, a1], na.rm = TRUE) <
                             mean(dev_matrix[, a2], na.rm = TRUE), a1, a2)
            cat(sprintf("  %-40s vs %-40s  p=%.f  (better: %s)\n", a1, a2, p, better))
            found <- TRUE
        }
    }
}
if (!found) cat("  None found at alpha =", alpha, "\n")

# --- 4. Box plot of deviations -----------------------------------------------
# Check if a package is installed before using it.
# Check whether the package ggplot2 is installed.
# quietly = TRUE -> suppresses messages (so no warning/output if it’s missing).
if (requireNamespace("ggplot2", quietly = TRUE)) {
    # Loads the package ggplot2 into the current R session.
    # Makes all its functions directly available.
    library(ggplot2)

    # ggplot(data, ...) -> starts a plot using your dataset data.
    # aes(...) (aesthetics):
    #   x = algorithm -> x-axis shows different algorithms.
    #   y = deviation_percentage -> y-axis shows percentage deviation.
    #   fill = start -> color of boxes depends on start (grouping variable).
    # This tells ggplot:
    # - which column goes on the x-axis -> algorithm.
    # - which column goes on the y-axis -> deviation_percentage.
    # This is about data and visual mapping, not labels.
    p <- ggplot(data, aes(x = algorithm, y = deviation_percentage, fill = start)) +
        # Adds a boxplot layer.
        # Shows distribution of deviation_percentage for each algorithm.
        # What label text is displayed.
        geom_boxplot() +
        labs(title = "LOP: % deviation from best known by algorithm",
             x = "Algorithm", 
             y = "% deviation") +
        theme(
            # Rotates x-axis text by 90 degrees.
            # hjust = 1 -> right-aligns labels.
            # Useful when algorithm names are long or overlapping.
            axis.text.x = element_text(angle = 90, hjust = 1, size = 8),
            # Increase margins (bottom, left, top, right)
            plot.margin = margin(t = 10, r = 10, b = 120, l = 10)
        )

    # Saves the plot p to a file:
    # File name: deviation_boxplot.pdf
    # Width: 12 inches
    # Height: 10 inches
    ggsave(paste0(directory, "\\deviation_boxplot_1.pdf"), p, width = 12, height = 10)

    cat("\nBoxplot saved to deviation_boxplot_1.pdf\n")
# Base R fallback
} else {
    # Starts writing all plots to a PDF file.
    # Open a graphics device.
    # This means: from now on, send all plots to this PDF file instead of the screen.
    pdf(paste0(directory, "\\deviation_boxplot_1.pdf"), width = 14, height = 10)
    # Increase margins (bottom, left, top, right)
    par(mar = c(12, 4, 4, 2))
    # Creates a boxplot of deviation_percentage grouped by algorithm.
    # las = 2 -> Rotates axis labels vertically.
    # cex.axis = 0.6 -> reduces the size of axis labels to 60% of default.
    # main = "LOP: % deviation from best known" -> sets the title of the plot.
    # ylab = "% deviation (lower = better)" -> labels the y-axis.
    boxplot(deviation_percentage ~ algorithm, data = data,
            las = 2, cex.axis = 0.6,
            main = "LOP: % deviation from best known",
            ylab = "% deviation")
    # Closes the PDF device, finalizing the file.
    # Closes the graphics device.
    # Done writing to this PDF, save it.
    dev.off()
    cat("\nBoxplot saved to deviation_boxplot_1.pdf\n")
}
