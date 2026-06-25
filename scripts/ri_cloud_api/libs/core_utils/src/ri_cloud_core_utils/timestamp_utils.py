def iso_str_to_date_str(iso_str: str) -> str:
    """
    Extract the date portion from an ISO 8601 string

    Handles formats like:
      '2018-01-01T00:00:00'
      '2018-01-01T00:00:00.000Z'
      '2018-01-01T00:00:00Z'
      '2018-01-01'

    Returns: 'YYYY-MM-DD'
    """
    # Split on 'T' and take the date portion
    return iso_str.split("T")[0]
