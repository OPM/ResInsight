---
layout: default
title: ResInsight &bull; Search
overview: true
---

<section>
  <div class="grid">
    <div class="whole">
      <article>
        <h1>Search for: <span id="search-title"></span></h1>
        <div id="search-results">
        </div>
      </article>
    </div>
  </div>
  <div class="clear"></div>
</section>

<script src="https://ajax.googleapis.com/ajax/libs/jquery/3.2.1/jquery.min.js"></script>
<script src="{{ site.baseurl }}/js/lunr.js"></script>
<script>
window.store = {
{% for p in site.pages %}
  {% if p.published %}
    "{{ p.url | slugify }}": {
      "title": "{{ p.title }}",
      "content": {{ p.content | strip_html | jsonify }},
      "html": {{ p.content | jsonify }},
      "url": "{{ p.url }}",
    },
  {% endif %}
{% endfor %}
};
window.baseurl = "{{ site.baseurl }}";
</script>
<script src="{{ site.baseurl }}/js/search.js"></script>
