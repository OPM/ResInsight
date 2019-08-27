(function() {
  function noResults() {
    var searchResults = document.getElementById("search-results");
    searchResults.innerHTML = "<div>No results found</div>";
  }

  function hilightText(regexes, text) {
    for (var i = 0; i < regexes.length; i++) {
      text = text.replace(regexes[i], "<span class='search-hilight'>$1</span>");
    }
    return text;
  }

  function matchRegexes(metadata) {
    var regexes = [];
    for (var kw in metadata) {
      regexes.push(new RegExp("\\b("+kw+"\\w*)", "mig"));
    }
    return regexes;
  }

  function getMatchingSubtitle(item, from) {
    var subtitle = $(from).prevAll(":header:first");
    if (subtitle.length > 0) {
      return {
        content: "<p class='search-result-subtitle'><a href='"+window.baseurl+item.url+"#"+subtitle[0].id+"'>"+$(subtitle[0]).text()+"</a></p>",
        id: subtitle[0].id
      };
    }
    if ($(from).parent().length === 0) {
      return null;
    } else {
      return getMatchingSubtitle(item, $(from).parent()[0]);
    }
  }

  function getMatchingText(regexes, item, paragraph, lastSubtitle) {
    var matchingText = "";
    var text = paragraph.text();
    for (var j = 0; j < regexes.length; j++) {
      var match = regexes[j].exec(text);
      if (match) {
        var subtitle = getMatchingSubtitle(item, paragraph);
        if (subtitle !== null && lastSubtitle != subtitle.id) {
          matchingText += subtitle.content;
          lastSubtitle = subtitle.id;
        }
        matchingText += "<p class='search-result-data'>" + hilightText(regexes, text) + "</p>";
        return {
          content: matchingText,
          lastSubtitle: lastSubtitle
        };
      }
    }
    return null;
  }

  function getItemText(item, metadata) {
    var itemText = "<div><h3 class='search-result-title'><a href='"+window.baseurl+item.url+"'>"+item.title+"</a></h3>";
    var dummy = document.createElement("div");
    dummy.innerHTML = item.html;
    var paragraphs = $(dummy).find("p, li, div, td");
    var lastSubtitle = null;
    var regexes = matchRegexes(metadata);
    for (var p = 0;  p < paragraphs.length; p++) {
      var matchingText = getMatchingText(regexes, item, $(paragraphs[p]), lastSubtitle);
      if (matchingText !== null) {
        itemText += matchingText.content;
        lastSubtitle = matchingText.lastSubtitle;
      }
    }
    itemText += "</div>";
    return itemText;
  }

  function displaySearchResults(term, results, store) {
    if (results.length === 0) {
      noResults();
    } else {
      var searchResults = document.getElementById("search-results");
      var append = "";
      for (var i = 0; i < results.length; i++) {
        var item = store[results[i].ref];
        append += getItemText(item, results[i].matchData.metadata);
      }
      searchResults.innerHTML = append;
    }
  }

  function getQueryVariable(variable) {
    var query = window.location.search.substring(1);
    var vars = query.split("&");
    for (var i = 0; i < vars.length; i++) {
      var pair = vars[i].split("=");

      if (pair[0] === variable) {
        return decodeURIComponent(pair[1].replace(/\+/g, "%20"));
      }
    }
  }

  function createIndex() {
    var index = lunr(function() {
      this.field("title");
      this.field("content");

      for (var key in window.store) {
        this.add({
          "id": key,
          "title": window.store[key].title,
          "content": window.store[key].content
        });
      }
    });
 
    return index;
  }

  var searchTerm = getQueryVariable("q");
  if (searchTerm) {
    document.getElementById("search-title").innerHTML = searchTerm;
  
    var index = createIndex();

    var results = index.search(searchTerm);
    displaySearchResults(searchTerm, results, window.store);
  } else {
    noResults();
  }
})();
