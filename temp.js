function callBack(i, s)
{
        var button = $('#switch'+i);
	var status = $('#switch'+i +' span');
	status.removeClass('loading');
        if (s==0){
                 button.removeClass('on');
                 status.text('OFF');
        }
        else
        {
                 button.addClass('on');
                 status.text('ON');
	}
}

$(function() {

	function update(i, s){
		$.ajax(
			{type: 'GET',
			 url: '/api/',
			 data: { 'i' : i.attr('id'), 's' : s  },
			 dataType: 'script',
			 complete: function() {

				if (s==0){
					$(this).addClass('off').removeClass('on');
                                	$(this).text('OFF');
				}
				else
				{
	                                $(this).addClass('on').removeClass('off');
        	                        $(this).text('ON');
				}

			    }
			}
		)
	}

	$('.switch').click(
		function (){
			console.log($(this).attr('id'));
			if ($(this).hasClass('on')){
				update($(this), 0);
			}
			else
			{
				update($(this), 1);
			}
			var status = $(this).children('span');
			status.addClass('loading');
		}
	);
});
