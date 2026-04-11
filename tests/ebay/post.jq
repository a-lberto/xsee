# Use the listings array as the root
.listings |= map(
  # Only attempt to modify if the URL exists and is a string
  if (.url | type) == "string" then
    .url |= gsub("\\?.*$"; "")
  else
    .
  end
)