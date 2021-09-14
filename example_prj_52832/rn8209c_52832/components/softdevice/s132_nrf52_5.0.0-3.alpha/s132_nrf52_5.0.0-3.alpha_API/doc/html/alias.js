
function getElementsByClass(searchClass,node,tag)
{
  var classElements = new Array();
  
  if(node == null)
  {
    node = document;
  }
  
  if(tag == null)
  {
    tag = '*';
  }
  
  var els = node.getElementsByTagName(tag);
  var elsLen = els.length;
  var pattern = new RegExp("(^|\\\\s)"+searchClass+"(\\\\s|$)");
  
  for(i = 0, j = 0; i < elsLen; i++)
  {
    if( pattern.test(els[i].className) )
    {
      classElements[j] = els[i];
      j++;
    }
  }
  
  return classElements;
}

function hover()
{
  this.item.parentNode.className = "gitem_cap_over"
}

function hout()
{
  this.item.parentNode.className = "gitem_cap_out"
}

/**
 * @gitem{3}
 * @gitem{ Path to image, Reference, Caption }
 * 
 * <div class="gitem"> @image html \1 \n 
 *  <span>
 *   <span class="gitem_cap_out">Figure </span>
 *   <span class="_gitem" >\2</span>
 *   <span> - \3</span>
 *  </span>
 * </div>
 *
 * @gref{1}
 * @gref{ Reference }
 *
 * <span class="gref"><span>Figure </span><span class="_gref"> \1 </span></span>
 *
 */
function common(sourceClass, refClass, type, number )
{
  var gitems = getElementsByClass(sourceClass,document, "span")

  for(i=0;i<gitems.length;i++ )
  {
    gitems[i].id = type+"_"+gitems[i].innerHTML.split(" ").join("");  // Assign ID to gitem
    gitems[i].innerHTML = (number++)                              // Assign unique running number to gitem
  }

  var grefs = getElementsByClass(refClass,document, "span")

  for(i=0;i<grefs.length;i++ )
  {
    var item_name = grefs[i].innerHTML.split(" ").join("");
    
    var item_id = type+"_"+item_name;
    
    var item = document.getElementById(item_id);
    
    if(item != null)
    {
      grefs[i].innerHTML = item.innerHTML;
      grefs[i].parentNode.onmouseover = hover
      grefs[i].parentNode.onmouseout = hout
      grefs[i].parentNode.item = item;
    }
    else
    {
      grefs[i].innerHTML = "Invalid Reference ("+item_name+")!"
    }   
  }
}

function gref()
{
  common("_img", "_iref", "img", 1)
  common("_table", "_tref", "table", 1)
}
