library(knitr)

listings <- list.files(pattern="\\.Rmd$")
defaults <- c("0.99", "1.0", "1.1")
known.variants <- list()

dest <- "compiled"
unlink(dest, recursive=TRUE)
dir.create(dest)

deposit_placeholder_spiel <- function(can.be.string, data.name) {
    if (.version != package_version("0.0")) {
        cat("`", data.name, "` may contain a `missing_placeholder` attribute.\n", sep="")
        if (.version == package_version("1.0")) {
            cat("This should be a scalar dataset of the same type class as `", data.name, "`, specifying the placeholder value used for all missing elements,
i.e., any elements in `", data.name, "` with the same value as the placeholder should be treated as missing.
Note that, for floating-point datasets, the placeholder itself may be NaN, so byte-wise comparison should be used when checking for missingness.", sep="")
        } else {
            msg <- paste0("This should be a scalar dataset of the same type as `", data.name, "`")
            if (can.be.string) {
                msg <- paste(msg, "(except for a string `", data.name, "`, in which case only the same datatype class is required).")
            } else {
                msg <- paste0(msg, ".");
            }
            msg <- paste(msg, "\nThe value of this dataset is the missing placeholder used for all missing elements,
i.e., any elements in `", data.name, "` with the same value as the placeholder should be treated as missing.
Note that, for floating-point datasets, the placeholder itself may be NaN; in this case, any NaN value in `", data.name, "` should be treated as missing.", sep="")
            cat(msg)
        }
    }
}

for (n in listings) {
    versions <- c(defaults, known.variants[[n]])
    odir <- file.path(dest, sub("\\.Rmd$", "", n))
    unlink(odir, recursive=TRUE)
    dir.create(odir)
    for (v in versions) {
        .version <- package_version(v)
        knitr::knit(n, file.path(odir, paste0(v, ".md")))
    }
}

file.copy("_general.md", dest)