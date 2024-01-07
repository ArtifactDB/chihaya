library(knitr)

listings <- list.files(pattern="\\.Rmd$")
defaults <- c("0.99", "1.0", "1.1")
known.variants <- list()

dest <- "compiled"
unlink(dest, recursive=TRUE)
dir.create(dest)

deposit_type_spiel <- function(can.be.string, indent) {
    if (.version > package_version("1.0")) {
        msg <- c('This should have a `delayed_type` scalar attribute of any string datatype, which should be one of the following:',
            '- `"integer"`, in which case `data` should have a datatype that fits into a 32-bit signed integer.',
            '- `"float"`, in which case `data` should have a datatype that fits into a 64-bit float.',
            '- `"boolean"`, in which case `data` should have a datatype that fits into a 8-bit signed integer.')
        msg <- c(msg, '- `"string"`, in which case `data` should have a datatype that can be represented by a UTF-8 encoded string.')
    } else {
        if (can.be.string) {
            msg <- "This can be either boolean, integer, float or string"
        } else {
            msg <- "This can be either boolean, integer or float"
        }
        msg <- paste0(msg, "; the exact type is left to the implementation.")
    }
    cat(paste(paste0(strrep(" ", indent), msg), collapse="\n"))
}

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
