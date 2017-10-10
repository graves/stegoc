var imgJscii = new Jscii({
    color: true,
    width: 190,
    el: document.getElementById('px'),
    fn: function(str) {
    document.body.innerHTML = "<pre>" + str + "<pre>";
  }
});
