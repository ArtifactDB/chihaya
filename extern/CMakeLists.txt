include(FetchContent)

FetchContent_Declare(
  ritsuko 
  GIT_REPOSITORY https://github.com/ArtifactDB/ritsuko
  GIT_TAG master #^0.6.0
)

FetchContent_MakeAvailable(ritsuko)
