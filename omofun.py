import requests
import json
head = {
    'user-agent': 'Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/86.0.4240.198 Safari/537.36',
    'Referer': '' }
def get_anime_id(name = None,num=0):
    url = 'http://103.91.210.141:2515/xgapp.php/v2/search'
    params = {"pg":1,"text":name}
    response_dict = (requests.get(url,params = params,headers=head)).json()
    id_list = []
    for i in range(len(response_dict['data'])):
        id_list.append(((response_dict['data'])[i])['vod_id'])
    for i in range(len(response_dict['data'])):
        print(i,((response_dict['data'])[i])['vod_name'])
    num = int(input('请输入序号:'))
    return id_list[num]

def get_url(id,episodes=0):
    detail_url  = 'http://103.91.210.141:2515/xgapp.php/v2/video_detail'
    response_url = ((((((requests.get(detail_url,headers=head,params={'id':id})).json())['data'])['vod_info'])['vod_url_with_player'])[0])['url']
    video_url = ((((((requests.get(detail_url,headers=head,params={'id':id})).json())['data'])['vod_info'])['vod_url_with_player'])[0])['parse_api'].split('?')[0]
    video_url = 'http://103.91.210.141:6100/jx.php'
    split_url = response_url.split('#')
    for i in range(len(split_url)):
        print(i,(split_url[i]).split('$')[0])
    episodes = int(input('请输入集数:'))
    true_url = ((requests.get(video_url,params={'url':(split_url[episodes].split('$')[1])})).json())['url']
    return true_url
print(get_url(get_anime_id(input('请输入要搜索的番剧:'))))