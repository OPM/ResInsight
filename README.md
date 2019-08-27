ResInsight
==========

3D viewer and post processing of reservoir models

# Procedure for editing in a fork
- Fork ResInsight into your personal repository
- Open Settings
    - Set gh-pages as default branch
    - Enable Git Hub Pages
- delete the CNAME file in the fork
- go to your_githubname.github.io/ResInsight and see your version of the site
- commit your changes, and they will appear on the site after a few seconds

# Edit of site layout
The layout of the site is mainly controlled by the css-files located in _includes/css.

## Puliblishing to main gh-pages
The history is not important, so publishing of the new site to ResInsight gh-pages is easiest done by manually copying your site changes into a single checkin on ResInsight/gh-pages. 

You will then have to delete all the files in ResInsight/gh-pages **exept** the .git directory and CNAME.
Then copy all the files from the new "site" **exept** the .git directory and CNAME.


# MathJax
The following script can be placed in top.html to enable parsing of math formulas.

```
  <script type="text/x-mathjax-config">
    MathJax.Hub.Config({
      messageStyle: "none",
      showProcessingMessages: "false",
      "HTML-CSS": {
        scale: 80
      },
      asciimath2jax: {
        delimiters: [['$','$']]
      }
  });
  </script>
  <script src="https://cdn.mathjax.org/mathjax/latest/MathJax.js?config=AM_CHTML" type="text/javascript">
</script>
```
