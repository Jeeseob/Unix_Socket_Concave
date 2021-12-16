package com.omok.unix_omok.Service;

import com.omok.unix_omok.model.OmokData;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;
import com.omok.unix_omok.repository.OmokDataRepository;

import java.util.List;

@Transactional
@Service
public class OmokDataService {

    private final OmokDataRepository omokDataRepository;

    public OmokDataService(OmokDataRepository omokDataRepository) {
        this.omokDataRepository = omokDataRepository;
    }

    public List<OmokData> findList() {
        return omokDataRepository.findAll();
    }
}