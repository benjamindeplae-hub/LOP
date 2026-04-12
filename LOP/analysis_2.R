# Expected CSV format:
# algorithm, cost, time_in_seconds, deviation_percentage

directory <- "C:\\Users\\benja\\OneDrive\\Bureaublad\\vub\\master_ai\\semester_2\\heuristic\\task\\LOP\\x64\\Debug"

# Usage in R:
# echo = FALSE -> R will not print the commands (red text).
# source("C:\\Users\\benja\\OneDrive\\Bureaublad\\vub\\master_ai\\semester_2\\heuristic\\task\\LOP\\LOP\\analysis_2.R", echo = FALSE)

# --- 0. Get all CSV files in the folder -------------------------------------
files <- list.files(paste0(directory, "\\implementation_2_results"), pattern = "\\.csv$", full.names = TRUE)
data <- do.call(rbind, lapply(files, function(f) {
    df <- read.csv(f, stringsAsFactors = FALSE)
    instance_name <- tools::file_path_sans_ext(basename(f))
    df$instance <- instance_name
    return(df)
}))

cat("Loaded", nrow(data), "rows covering",
    length(unique(data$instance)), "instances and",
    length(unique(data$algorithm)), "algorithms.\n\n")

# --- 1. Summary statistics per algorithm -------------------------------------
cat("=== Average percentage deviation from best known (%) ===\n")
avg_dev <- aggregate(deviation_percentage ~ algorithm, data = data, FUN = mean)
avg_dev <- avg_dev[order(avg_dev$deviation_percentage), ]
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

dev_matrix <- matrix(NA, nrow = length(instances), ncol = n_alg,
                     dimnames = list(instances, algorithms))

for (inst in instances) {
    for (alg in algorithms) {
        val <- data$deviation_percentage[data$instance == inst & data$algorithm == alg]
        if (length(val) == 1)
            dev_matrix[inst, alg] <- val
    }
}

cat("\n=== Pairwise Student t-test p-values (paired, two-sided) ===\n")
ttest_p <- matrix(NA, n_alg, n_alg, dimnames = list(algorithms, algorithms))
for (a1 in algorithms) {
    for (a2 in algorithms) {
        if (a1 == a2) { ttest_p[a1, a2] <- 1; next }
        x <- dev_matrix[, a1]
        y <- dev_matrix[, a2]
        ok <- complete.cases(x, y)
        if (sum(ok) >= 2) {
            res <- t.test(x[ok], y[ok], paired = TRUE)
            ttest_p[a1, a2] <- res$p.value
        }
    }
}
print(ttest_p)

cat("\n=== Pairwise Wilcoxon signed-rank test p-values (paired, two-sided) ===\n")
wilcox_p <- matrix(NA, n_alg, n_alg, dimnames = list(algorithms, algorithms))
for (a1 in algorithms) {
    for (a2 in algorithms) {
        if (a1 == a2) { wilcox_p[a1, a2] <- 1; next }
        x <- dev_matrix[, a1]
        y <- dev_matrix[, a2]
        ok <- complete.cases(x, y)
        if (sum(ok) >= 2) {
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
        if (a1 >= a2) next
        p <- wilcox_p[a1, a2]
        if (!is.na(p) && p < alpha) {
            better <- ifelse(mean(dev_matrix[, a1], na.rm = TRUE) <
                             mean(dev_matrix[, a2], na.rm = TRUE), a1, a2)
            cat(sprintf("  %-40s vs %-40s  p=%.f  (better: %s)\n", a1, a2, p, better))
            found <- TRUE
        }
    }
}
if (!found) cat("  None found at alpha =", alpha, "\n")

# --- 4. Box plot of deviations -----------------------------------------------
if (requireNamespace("ggplot2", quietly = TRUE)) {
    library(ggplot2)
    p <- ggplot(data, aes(x = algorithm, y = deviation_percentage, fill = start)) +
        geom_boxplot() +
        labs(title = "LOP: % deviation from best known by algorithm",
             x = "Algorithm", 
             y = "% deviation") +
        theme(
            axis.text.x = element_text(angle = 90, hjust = 1, size = 8),
            plot.margin = margin(t = 10, r = 10, b = 120, l = 10)
        )
    ggsave(paste0(directory, "\\deviation_boxplot_2.pdf"), p, width = 12, height = 10)
    cat("\nBoxplot saved to deviation_boxplot_2.pdf\n")
} else {
    pdf(paste0(directory, "\\deviation_boxplot_2.pdf"), width = 14, height = 10)
    par(mar = c(12, 4, 4, 2))
    boxplot(deviation_percentage ~ algorithm, data = data,
            las = 2, cex.axis = 0.6,
            main = "LOP: % deviation from best known",
            ylab = "% deviation (lower = better)")
    dev.off()
    cat("\nBoxplot saved to deviation_boxplot_2.pdf\n")
}
