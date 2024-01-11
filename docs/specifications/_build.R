library(knitr)

listings <- list.files(pattern="\\.Rmd$")

all.versions <- c("0.99", "1.0", "1.1")
startpoint <- list()
endpoint <- list(
    external_hdf5.Rmd="1.0"
)

dest <- "compiled"
unlink(dest, recursive=TRUE)
dir.create(dest)

deposit_placeholder_spiel <- function(can.be.string, data.name) {
    if (.version != package_version("0.0")) {
        cat("`", data.name, "` may contain a `missing_placeholder` attribute.\n", sep="")
        if (.version == package_version("1.0")) {
            cat("This should be a scalar dataset of the same type class as `", data.name, "`, specifying the placeholder value used for all missing elements,
i.e., any elements in `", data.name, "` with the same value as the placeholder should be treated as missing.", sep="")
        } else {
            msg <- paste0("This should be a scalar dataset of the same type as `", data.name, "`")
            if (can.be.string) {
                msg <- paste(msg, "(except for a string `", data.name, "`, in which case only the same datatype class is required).")
            } else {
                msg <- paste0(msg, ".");
            }
            msg <- paste(msg, "\nThe value of this dataset is the missing placeholder used for all missing elements,
i.e., any elements in `", data.name, "` with the same value as the placeholder should be treated as missing.", sep="")
            cat(msg)
        }
    }
}


for (n in listings) {
    versions <- all.versions
    if (n %in% names(endpoint)) {
        end <- endpoint[[n]]
        versions <- versions[1:which(versions == end)]
    }
    if (n %in% names(startpoint)) {
        start <- startpoint[[n]]
        versions <- versions[which(versions == start):length(versions)]
    }

    oname <- sub("\\.Rmd$", "", n)
    for (v in versions) {
        vdir <- file.path(dest, v)
        dir.create(vdir, showWarnings=FALSE)
        .version <- package_version(v)
        knitr::knit(n, file.path(vdir, paste0(oname, ".md")))
    }
}
